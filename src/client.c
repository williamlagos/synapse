#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#define DEFAULT_SWITCHER_PATH "/tmp/synapse.access"
#define CLIENT_MAX_CMD 256

static uv_loop_t* loop;
static uv_pipe_t client_pipe;
static uv_connect_t connect_req;
static char command[CLIENT_MAX_CMD];

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

static void on_close(uv_handle_t* handle)
{
    (void) handle;
}

static void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
    (void) handle;
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

static void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
    if (nread > 0) {
        fwrite(buf->base, 1, (size_t) nread, stdout);
        uv_close((uv_handle_t*) stream, on_close);
    } else if (nread < 0) {
        if (nread != UV_EOF) {
            fprintf(stderr, "synapse-client: read error: %s\n", uv_strerror((int) nread));
        }
        uv_close((uv_handle_t*) stream, on_close);
    }
    if (buf->base) {
        free(buf->base);
    }
}

static void on_write(uv_write_t* req, int status)
{
    write_req_t* wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
    if (status < 0) {
        fprintf(stderr, "synapse-client: write error: %s\n", uv_strerror(status));
        return;
    }
    uv_read_start(req->handle, alloc_buffer, on_read);
}

static void on_connect(uv_connect_t* req, int status)
{
    if (status < 0) {
        fprintf(stderr, "synapse-client: cannot connect to %s: %s\n",
                DEFAULT_SWITCHER_PATH, uv_strerror(status));
        return;
    }

    write_req_t* wr = (write_req_t*) malloc(sizeof(write_req_t));
    char cmd_buf[CLIENT_MAX_CMD + 2];
    snprintf(cmd_buf, sizeof(cmd_buf), "%s\n", command);
    wr->buf = uv_buf_init(strdup(cmd_buf), strlen(cmd_buf));
    uv_write((uv_write_t*) wr, req->handle, &wr->buf, 1, on_write);
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: synapse-client <command>\n");
        fprintf(stderr, "Commands:\n");
        fprintf(stderr, "  status  - Show synapse service status\n");
        fprintf(stderr, "  stop    - Stop the synapse service\n");
        fprintf(stderr, "  reload  - Reload configuration\n");
        return EXIT_FAILURE;
    }

    strncpy(command, argv[1], CLIENT_MAX_CMD - 1);
    command[CLIENT_MAX_CMD - 1] = '\0';

    loop = uv_default_loop();
    uv_pipe_init(loop, &client_pipe, 0);
    uv_pipe_connect(&connect_req, &client_pipe, DEFAULT_SWITCHER_PATH, on_connect);

    return uv_run(loop, UV_RUN_DEFAULT);
}
