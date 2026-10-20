#include "utils.h"
#include "switcher.h"

context_t* contexts = NULL;
int active_contexts = 0;
char current_cfgpath[256] = DEFAULT_CONFIGURATION_PATH;
uv_loop_t* loop = NULL;
int64_t counter = 0;

static char bind_host[64] = DEFAULT_SWITCHER_HOST;
static uint16_t bind_port = DEFAULT_SWITCHER_PORT;

void main_cycle(uv_timer_t* handle)
{
    (void) handle;
    for (int i = 0; i < active_contexts; i++) {
        context_t* context = &contexts[i];
        config_t* r = &(context->config);
        if (r->name[0] == '\0' || r->n_sensors <= 0) {
            continue;
        }
        fprintf(stderr, "Starting configuration for %s...\n", r->name);
        context->n_workers = r->n_sensors;
        if (context->workers == NULL) {
            context->workers = calloc((size_t) context->n_workers, sizeof(uv_work_t));
        }
        for (int s = 0; s < r->n_sensors; s++) {
            uv_work_t* worker_handle = &(context->workers[s]);
            if (worker_handle->data == NULL) {
                worker_t* worker = (worker_t*) calloc(1, sizeof(worker_t));
                worker->status = -10;
                worker->worker_n = s;
                if (r->state == NULL) {
                    worker->worker_args = strdup("");
                } else if (r->state_args == NULL || r->state_args[0] == '\0') {
                    worker->worker_args = strdup(r->state);
                } else {
                    size_t arg_len = strlen(r->state) + strlen(r->state_args) + 2;
                    worker->worker_args = (char*) malloc(arg_len);
                    snprintf(worker->worker_args, arg_len, "%s %s", r->state, r->state_args);
                }
                strncpy(worker->worker_name, context->config.sensors[s], sizeof(worker->worker_name) - 1);
                context->workers[s].data = (void*) worker;
            } else {
                int status = ((worker_t*) worker_handle->data)->status;
                fprintf(stderr, "Status worker: %d\n", status);
                if (status == SUCCESS_STATUS) {
                    fprintf(stderr, "Condition met: command: %s %s starting\n",
                            r->command ? r->command : "",
                            r->command_args ? r->command_args : "");
                    async_start_process(context, i);
                } else if (status == FAILURE_STATUS) {
                    fprintf(stderr, "Condition not met: command: %s %s not starting\n",
                            r->command ? r->command : "",
                            r->command_args ? r->command_args : "");
                }
            }
            fprintf(stderr, "Sensor: %s activated\n", r->sensors[s]);
            async_schedule_sensor(context, s);
        }
    }
}

void signal_handler(uv_signal_t *req, int signum)
{
    (void) signum;
    fprintf(stdout, "Stopping synapse service.\n");
    stop_switcher();
    uv_signal_stop(req);
    uv_stop(loop);
}

int event_loop(void)
{
    loop = uv_default_loop();

    uv_signal_t sigint, sigterm;
    uv_signal_init(loop, &sigint);
    uv_signal_start(&sigint, signal_handler, SIGINT);
    uv_signal_init(loop, &sigterm);
    uv_signal_start(&sigterm, signal_handler, SIGTERM);

    uv_timer_t timer_req;
    uv_timer_init(loop, &timer_req);
    uv_timer_start(&timer_req, main_cycle, 1000, 2000);

    start_switcher(loop, bind_host, bind_port);

    return uv_run(loop, UV_RUN_DEFAULT);
}

int main(int argc, char** argv)
{
    int state = BACKGROUND;
    int n_contexts = DEFAULT_MAX_CONTEXT;
    char logpath[256] = DEFAULT_LOG_PATH;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'f':
                    fprintf(stdout, "Starting synapse service on foreground.\n");
                    state = FOREGROUND;
                    break;
                case 'l':
                    if (i + 1 < argc) {
                        strncpy(logpath, argv[++i], sizeof(logpath) - 1);
                    }
                    break;
                case 'c':
                    if (i + 1 < argc) {
                        strncpy(current_cfgpath, argv[++i], sizeof(current_cfgpath) - 1);
                    }
                    break;
                case 'n':
                    if (i + 1 < argc) {
                        n_contexts = atoi(argv[++i]);
                    }
                    break;
                case 'b':
                    if (i + 1 < argc) {
                        strncpy(bind_host, argv[++i], sizeof(bind_host) - 1);
                    }
                    break;
                case 'p':
                    if (i + 1 < argc) {
                        bind_port = (uint16_t) atoi(argv[++i]);
                    }
                    break;
                default:
                    fprintf(stderr, "Usage: %s [-f] [-c <cfg>] [-l <log>] [-n <contexts>] [-b <host>] [-p <port>]\n", argv[0]);
                    exit(EXIT_FAILURE);
            }
        }
    }

    if (state == BACKGROUND) {
        pid_t process_id = fork();
        if (process_id < 0) exit(EXIT_FAILURE);
        if (process_id > 0) {
            fprintf(stderr, "Starting synapse service at %d.\n", process_id);
            exit(EXIT_SUCCESS);
        }
        umask(0);
        if (setsid() < 0) exit(EXIT_FAILURE);
    }

    if (strcmp(logpath, "-") != 0) {
        freopen(logpath, "a", stderr);
    }
    fprintf(stderr, "Starting reading configuration on %s.\n", current_cfgpath);
    contexts = (context_t*) calloc((size_t) n_contexts, sizeof(context_t));
    load_config(current_cfgpath, contexts);
    return event_loop();
}
