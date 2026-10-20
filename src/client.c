#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <uv.h>
#include <uv_sockets.h>

#define DEFAULT_SWITCHER_PATH "/tmp/synapse.access"
#define DEFAULT_SWITCHER_PORT 9460
#define CLIENT_MAX_CMD 256

static uv_loop_t* loop;
static uv_pipe_t client_pipe;
static uv_connect_t connect_req;
static uvsock_client_t tcp_client;
static char command[CLIENT_MAX_CMD];
static char socket_path[256] = DEFAULT_SWITCHER_PATH;
static const char* remote_host = NULL;
static uint16_t remote_port = DEFAULT_SWITCHER_PORT;

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

/* -------------------------------------------------------------------------
 * Local Unix Domain Pipe client callbacks
 * ---------------------------------------------------------------------- */

static void on_pipe_close(uv_handle_t* handle)
{
    (void) handle;
}

static void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
    (void) handle;
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

static void on_pipe_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
    if (nread > 0) {
        fwrite(buf->base, 1, (size_t) nread, stdout);
        uv_close((uv_handle_t*) stream, on_pipe_close);
    } else if (nread < 0) {
        if (nread != UV_EOF) {
            fprintf(stderr, "synapse-client: read error: %s\n", uv_strerror((int) nread));
        }
        uv_close((uv_handle_t*) stream, on_pipe_close);
    }
    if (buf->base) {
        free(buf->base);
    }
}

static void on_pipe_write(uv_write_t* req, int status)
{
    uv_stream_t* handle = req->handle;
    write_req_t* wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
    if (status < 0) {
        fprintf(stderr, "synapse-client: write error: %s\n", uv_strerror(status));
        return;
    }
    uv_read_start(handle, alloc_buffer, on_pipe_read);
}

static void on_pipe_connect(uv_connect_t* req, int status)
{
    if (status < 0) {
        fprintf(stderr, "synapse-client: cannot connect to %s: %s\n",
                socket_path, uv_strerror(status));
        return;
    }

    write_req_t* wr = (write_req_t*) malloc(sizeof(write_req_t));
    char cmd_buf[CLIENT_MAX_CMD + 2];
    snprintf(cmd_buf, sizeof(cmd_buf), "%s\n", command);
    wr->buf = uv_buf_init(strdup(cmd_buf), (unsigned int) strlen(cmd_buf));
    uv_write((uv_write_t*) wr, req->handle, &wr->buf, 1, on_pipe_write);
}

/* -------------------------------------------------------------------------
 * Remote TCP client callbacks via libsockets (uv_sockets.h)
 * ---------------------------------------------------------------------- */

static void on_tcp_read(uvsock_client_t* client, const char* data, ssize_t length)
{
    if (length > 0 && data != NULL) {
        fwrite(data, 1, (size_t) length, stdout);
    } else if (length < 0) {
        fprintf(stderr, "synapse-client: tcp read error: %s\n", uv_strerror((int) length));
    }
    uvsock_client_close(client, NULL);
}

static void on_tcp_write(uvsock_client_t* client, int status)
{
    if (status < 0) {
        fprintf(stderr, "synapse-client: tcp write error: %s\n", uv_strerror(status));
        uvsock_client_close(client, NULL);
        return;
    }
    uvsock_read_start(client, on_tcp_read);
}

static void on_tcp_connect(uvsock_client_t* client, int status)
{
    if (status < 0) {
        fprintf(stderr, "synapse-client: cannot connect to %s:%u: %s\n",
                remote_host, (unsigned) remote_port, uv_strerror(status));
        return;
    }

    char cmd_buf[CLIENT_MAX_CMD + 2];
    snprintf(cmd_buf, sizeof(cmd_buf), "%s\n", command);
    uvsock_write(client, cmd_buf, strlen(cmd_buf), on_tcp_write);
}

static void print_usage(const char* prog)
{
    fprintf(stderr, "Usage: %s [-h <host>] [-p <port>] [-s <unix_socket>] <command>\n", prog);
    fprintf(stderr, "Commands:\n");
    fprintf(stderr, "  status  - Show synapse service status\n");
    fprintf(stderr, "  ping    - Health-check ping (returns pong)\n");
    fprintf(stderr, "  list    - List configured contexts and sensor states\n");
    fprintf(stderr, "  reload  - Reload configuration file\n");
    fprintf(stderr, "  stop    - Stop the synapse service\n");
}

int main(int argc, char** argv)
{
    int cmd_index = -1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
            remote_host = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            remote_port = (uint16_t) atoi(argv[++i]);
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            strncpy(socket_path, argv[++i], sizeof(socket_path) - 1);
            socket_path[sizeof(socket_path) - 1] = '\0';
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (argv[i][0] != '-') {
            cmd_index = i;
            break;
        } else {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (cmd_index < 0) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    strncpy(command, argv[cmd_index], CLIENT_MAX_CMD - 1);
    command[CLIENT_MAX_CMD - 1] = '\0';

    loop = uv_default_loop();

    if (remote_host != NULL) {
        if (uvsock_client_init(&tcp_client, loop) != 0 ||
            uvsock_client_connect(&tcp_client, remote_host, remote_port, on_tcp_connect) != 0) {
            fprintf(stderr, "synapse-client: failed to initialize TCP connection to %s:%u\n",
                    remote_host, (unsigned) remote_port);
            return EXIT_FAILURE;
        }
        return uvsock_run(loop);
    }

    uv_pipe_init(loop, &client_pipe, 0);
    uv_pipe_connect(&connect_req, &client_pipe, socket_path, on_pipe_connect);

    return uv_run(loop, UV_RUN_DEFAULT);
}
