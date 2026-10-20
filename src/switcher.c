#include "switcher.h"
#include "utils.h"

#define SWITCHER_BACKLOG 128
#define SWITCHER_MAX_CMD 256
#define SWITCHER_MAX_RESP 2048

static uv_pipe_t server_pipe;
static uvsock_server_t tcp_server;
static int tcp_enabled = 0;
static uint16_t active_tcp_port = 0;
static uv_loop_t* switcher_loop;

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

static void handle_switcher_command(const char* raw_cmd, char* resp, size_t resp_size)
{
    char command[SWITCHER_MAX_CMD];
    size_t len = strlen(raw_cmd);
    if (len >= sizeof(command)) {
        len = sizeof(command) - 1;
    }
    memcpy(command, raw_cmd, len);
    command[len] = '\0';

    while (len > 0 && (command[len - 1] == '\n' || command[len - 1] == '\r' || command[len - 1] == ' ')) {
        command[--len] = '\0';
    }

    if (strcmp(command, "status") == 0) {
        snprintf(resp, resp_size,
                 "synapse: running (contexts=%d, unix=%s, tcp_port=%u)\n",
                 active_contexts, DEFAULT_SWITCHER_PATH, (unsigned) active_tcp_port);
    } else if (strcmp(command, "ping") == 0) {
        snprintf(resp, resp_size, "synapse: pong\n");
    } else if (strcmp(command, "list") == 0) {
        size_t offset = 0;
        offset += (size_t) snprintf(resp + offset, resp_size - offset,
                                    "contexts (%d):\n", active_contexts);
        for (int i = 0; i < active_contexts && offset < resp_size - 64; i++) {
            config_t* cfg = &contexts[i].config;
            int wstatus = -1;
            if (contexts[i].workers && contexts[i].workers[0].data) {
                wstatus = ((worker_t*) contexts[i].workers[0].data)->status;
            }
            offset += (size_t) snprintf(
                resp + offset,
                resp_size - offset,
                "  [%s] command=\"%s %s\" state=\"%s %s\" last_status=%d\n",
                cfg->name[0] ? cfg->name : "unnamed",
                cfg->command ? cfg->command : "",
                cfg->command_args ? cfg->command_args : "",
                cfg->state ? cfg->state : "",
                cfg->state_args ? cfg->state_args : "",
                wstatus);
        }
    } else if (strcmp(command, "stop") == 0) {
        snprintf(resp, resp_size, "synapse: stopping\n");
        uv_stop(switcher_loop);
    } else if (strcmp(command, "reload") == 0) {
        if (current_cfgpath[0] != '\0' && contexts != NULL) {
            load_config(current_cfgpath, contexts);
        }
        snprintf(resp, resp_size, "synapse: reloaded (%d contexts)\n", active_contexts);
    } else {
        snprintf(resp, resp_size, "synapse: unknown command: %s\n", command);
    }
}

/* -------------------------------------------------------------------------
 * Local Unix Domain Pipe handlers
 * ---------------------------------------------------------------------- */

static void on_pipe_client_close(uv_handle_t* handle)
{
    free(handle);
}

static void on_pipe_write(uv_write_t* req, int status)
{
    uv_stream_t* handle = req->handle;
    write_req_t* wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
    if (status < 0) {
        fprintf(stderr, "Switcher pipe write error: %s\n", uv_strerror(status));
    }
    if (!uv_is_closing((uv_handle_t*) handle)) {
        uv_close((uv_handle_t*) handle, on_pipe_client_close);
    }
}

static void send_pipe_response(uv_stream_t* client, const char* response)
{
    write_req_t* wr = (write_req_t*) malloc(sizeof(write_req_t));
    wr->buf = uv_buf_init(strdup(response), (unsigned int) strlen(response));
    uv_write((uv_write_t*) wr, client, &wr->buf, 1, on_pipe_write);
}

static void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
    (void) handle;
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

static void on_pipe_read(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf)
{
    if (nread > 0) {
        char raw[SWITCHER_MAX_CMD];
        size_t len = (size_t) nread < sizeof(raw) - 1 ? (size_t) nread : sizeof(raw) - 1;
        memcpy(raw, buf->base, len);
        raw[len] = '\0';

        char resp[SWITCHER_MAX_RESP];
        handle_switcher_command(raw, resp, sizeof(resp));
        send_pipe_response(client, resp);
    } else if (nread < 0) {
        if (nread != UV_EOF) {
            fprintf(stderr, "Switcher pipe read error: %s\n", uv_strerror((int) nread));
        }
        uv_close((uv_handle_t*) client, on_pipe_client_close);
    }
    if (buf->base) {
        free(buf->base);
    }
}

static void on_new_pipe_connection(uv_stream_t* server, int status)
{
    if (status < 0) {
        fprintf(stderr, "Switcher pipe connection error: %s\n", uv_strerror(status));
        return;
    }

    uv_pipe_t* client = (uv_pipe_t*) malloc(sizeof(uv_pipe_t));
    uv_pipe_init(switcher_loop, client, 0);

    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        uv_read_start((uv_stream_t*) client, alloc_buffer, on_pipe_read);
    } else {
        uv_close((uv_handle_t*) client, on_pipe_client_close);
    }
}

/* -------------------------------------------------------------------------
 * Remote TCP handlers via libsockets (uv_sockets.h)
 * ---------------------------------------------------------------------- */

static void on_tcp_client_closed(uvsock_client_t* client)
{
    free(client);
}

static void on_tcp_write_done(uvsock_client_t* client, int status)
{
    if (status < 0) {
        fprintf(stderr, "Switcher TCP write error: %s\n", uv_strerror(status));
    }
    uvsock_client_close(client, on_tcp_client_closed);
}

static void on_tcp_read(uvsock_client_t* client, const char* data, ssize_t length)
{
    if (length > 0 && data != NULL) {
        char raw[SWITCHER_MAX_CMD];
        size_t len = (size_t) length < sizeof(raw) - 1 ? (size_t) length : sizeof(raw) - 1;
        memcpy(raw, data, len);
        raw[len] = '\0';

        char resp[SWITCHER_MAX_RESP];
        handle_switcher_command(raw, resp, sizeof(resp));
        uvsock_write(client, resp, strlen(resp), on_tcp_write_done);
    } else {
        uvsock_client_close(client, on_tcp_client_closed);
    }
}

static void on_new_tcp_connection(uvsock_server_t* server, uvsock_client_t* client, int status)
{
    (void) server;
    if (status < 0 || client == NULL) {
        fprintf(stderr, "Switcher TCP connection error: %s\n", uv_strerror(status));
        if (client) {
            free(client);
        }
        return;
    }
    uvsock_read_start(client, on_tcp_read);
}

/* -------------------------------------------------------------------------
 * Lifecycle
 * ---------------------------------------------------------------------- */

void start_switcher(uv_loop_t* lp, const char* tcp_host, uint16_t tcp_port)
{
    switcher_loop = lp;

    uv_fs_t unlink_req;
    uv_fs_unlink(lp, &unlink_req, DEFAULT_SWITCHER_PATH, NULL);
    uv_fs_req_cleanup(&unlink_req);

    uv_pipe_init(lp, &server_pipe, 0);

    if (uv_pipe_bind(&server_pipe, DEFAULT_SWITCHER_PATH) == 0 &&
        uv_listen((uv_stream_t*) &server_pipe, SWITCHER_BACKLOG, on_new_pipe_connection) == 0) {
        fprintf(stderr, "Switcher listening on unix:%s\n", DEFAULT_SWITCHER_PATH);
    } else {
        fprintf(stderr, "Switcher: failed to listen on unix:%s\n", DEFAULT_SWITCHER_PATH);
    }

    if (tcp_port > 0) {
        if (uvsock_server_init(&tcp_server, lp) == 0 &&
            uvsock_server_bind(&tcp_server, tcp_host, tcp_port) == 0 &&
            uvsock_server_listen(&tcp_server, SWITCHER_BACKLOG, on_new_tcp_connection) == 0) {
            tcp_enabled = 1;
            active_tcp_port = tcp_port;
            fprintf(stderr, "Switcher listening on tcp://%s:%u (via libsockets)\n",
                    tcp_host ? tcp_host : "0.0.0.0", (unsigned) tcp_port);
        } else {
            fprintf(stderr, "Switcher: failed to bind/listen on tcp://%s:%u\n",
                    tcp_host ? tcp_host : "0.0.0.0", (unsigned) tcp_port);
        }
    }
}

void stop_switcher(void)
{
    uv_close((uv_handle_t*) &server_pipe, NULL);
    uv_fs_t unlink_req;
    uv_fs_unlink(switcher_loop, &unlink_req, DEFAULT_SWITCHER_PATH, NULL);
    uv_fs_req_cleanup(&unlink_req);

    if (tcp_enabled) {
        uvsock_server_close(&tcp_server);
        tcp_enabled = 0;
        active_tcp_port = 0;
    }
}
