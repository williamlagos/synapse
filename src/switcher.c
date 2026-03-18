#include "switcher.h"

#define SWITCHER_BACKLOG 128
#define SWITCHER_MAX_CMD 256
#define SWITCHER_MAX_RESP 512

static uv_pipe_t server_pipe;
static uv_loop_t* switcher_loop;

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

static void on_client_close(uv_handle_t* handle)
{
    free(handle);
}

static void on_write(uv_write_t* req, int status)
{
    write_req_t* wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
    if (status < 0) {
        fprintf(stderr, "Switcher write error: %s\n", uv_strerror(status));
    }
}

static void send_response(uv_stream_t* client, const char* response)
{
    write_req_t* wr = (write_req_t*) malloc(sizeof(write_req_t));
    wr->buf = uv_buf_init(strdup(response), strlen(response));
    uv_write((uv_write_t*) wr, client, &wr->buf, 1, on_write);
}

static void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
    (void) handle;
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

static void on_read(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf)
{
    if (nread > 0) {
        char command[SWITCHER_MAX_CMD];
        size_t len = (size_t) nread < sizeof(command) - 1 ? (size_t) nread : sizeof(command) - 1;
        strncpy(command, buf->base, len);
        command[len] = '\0';

        /* Strip trailing newline */
        size_t cmd_len = strlen(command);
        if (cmd_len > 0 && command[cmd_len - 1] == '\n') {
            command[cmd_len - 1] = '\0';
        }

        if (strcmp(command, "status") == 0) {
            send_response(client, "synapse: running\n");
        } else if (strcmp(command, "stop") == 0) {
            send_response(client, "synapse: stopping\n");
            uv_stop(switcher_loop);
        } else if (strcmp(command, "reload") == 0) {
            send_response(client, "synapse: reloaded\n");
        } else {
            char resp[SWITCHER_MAX_RESP];
            snprintf(resp, sizeof(resp), "synapse: unknown command: %s\n", command);
            send_response(client, resp);
        }
    } else if (nread < 0) {
        if (nread != UV_EOF) {
            fprintf(stderr, "Switcher read error: %s\n", uv_strerror((int) nread));
        }
        uv_close((uv_handle_t*) client, on_client_close);
    }
    if (buf->base) {
        free(buf->base);
    }
}

static void on_new_connection(uv_stream_t* server, int status)
{
    if (status < 0) {
        fprintf(stderr, "Switcher connection error: %s\n", uv_strerror(status));
        return;
    }

    uv_pipe_t* client = (uv_pipe_t*) malloc(sizeof(uv_pipe_t));
    uv_pipe_init(switcher_loop, client, 0);

    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        uv_read_start((uv_stream_t*) client, alloc_buffer, on_read);
    } else {
        uv_close((uv_handle_t*) client, on_client_close);
    }
}

void start_switcher(uv_loop_t* lp)
{
    switcher_loop = lp;

    uv_fs_t unlink_req;
    uv_fs_unlink(lp, &unlink_req, DEFAULT_SWITCHER_PATH, NULL);
    uv_fs_req_cleanup(&unlink_req);

    uv_pipe_init(lp, &server_pipe, 0);

    if (uv_pipe_bind(&server_pipe, DEFAULT_SWITCHER_PATH)) {
        fprintf(stderr, "Switcher: failed to bind on %s\n", DEFAULT_SWITCHER_PATH);
        return;
    }

    if (uv_listen((uv_stream_t*) &server_pipe, SWITCHER_BACKLOG, on_new_connection)) {
        fprintf(stderr, "Switcher: failed to listen on %s\n", DEFAULT_SWITCHER_PATH);
        return;
    }

    fprintf(stderr, "Switcher listening on %s\n", DEFAULT_SWITCHER_PATH);
}

void stop_switcher()
{
    uv_close((uv_handle_t*) &server_pipe, NULL);
    uv_fs_t unlink_req;
    uv_fs_unlink(switcher_loop, &unlink_req, DEFAULT_SWITCHER_PATH, NULL);
    uv_fs_req_cleanup(&unlink_req);
}
