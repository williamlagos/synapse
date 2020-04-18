// #include "syn.h"
#include "utils.h"

#define MAX_CONFIGS 2

void main_cycle() {
	for (int i = 0; i < MAX_CONFIGS; i++) {
        context_t* context = &contexts[i];
        config_t* r = &(context->config);
        fprintf(stderr, "Starting configuration for %s...\n", r->name);
        context->n_workers = r->n_sensors;
        if (context->workers == NULL) {
            context->workers = calloc(sizeof(uv_work_t), context->n_workers);
        }
        for (int s = 0; s < r->n_sensors; s++) {
            uv_work_t* worker_handle = (uv_work_t*) &(context->workers[s]);
            if ((worker_t*) worker_handle->data == NULL) {
                worker_t* worker = (worker_t*) malloc(sizeof(worker_t));
                worker->status = -10;
	            worker->worker_n = s;
                // Verify if worker has arguments to pass
                if (r->state_args == NULL || r->state == NULL) {
                    worker->worker_args = (char*) malloc(sizeof(char));
                    strcpy(worker->worker_args, " ");
                } else {
                    worker->worker_args = (char*) malloc(sizeof(r->state) + sizeof(r->state_args));
                    sprintf(worker->worker_args, "%s %s", r->state, r->state_args);
                }
	            strcpy(worker->worker_name, context->config.sensors[s]);
	            context->workers[s].data = (void*) worker;     
            } else {
                int status = ((worker_t*) worker_handle->data)->status;
                fprintf(stderr, "Status worker: %d\n", status);
                if (status == SUCCESS_STATUS) {
                    fprintf(stderr, "Condition met: command: %s %s starting\n", r->command, r->command_args);
                    async_start_process(context, i);
                } else if (status == FAILURE_STATUS) {
                    fprintf(stderr, "Condition not met: command: %s %s not starting\n", r->command, r->command_args);
                }
            }
            fprintf(stderr, "Sensor: %s activated\n", r->sensors[s]);
            async_schedule_sensor(context, s);
        }
	}

}

// Signal handling basics for dummy cycle
void signal_handler(uv_signal_t *req, int signum)
{
    fprintf(stdout, "Stopping synapse service.\n");
    uv_signal_stop(req);
    uv_stop(loop);
}

int event_loop(int argc, char** argv) {
    loop = uv_default_loop();

    uv_signal_t sig;
    uv_signal_init(loop, &sig);
    uv_signal_start(&sig, signal_handler, SIGINT);

    fibonacci_cycle(loop);

    uv_idle_t idler;
    counter = 0;
    uv_idle_init(loop, &idler);
    uv_idle_start(&idler, idle);

    uv_timer_t timer_req;
    uv_timer_init(loop, &timer_req);
    uv_timer_start(&timer_req, main_cycle, 5000, 2000);

    return uv_run(loop, UV_RUN_DEFAULT);
}

int main(int argc, char** argv) {
    int state = BACKGROUND;
    int n_contexts = DEFAULT_MAX_CONTEXT;
    char logpath[128] = DEFAULT_LOG_PATH;
    char cfgpath[128] = DEFAULT_CONFIGURATION_PATH;  
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch(argv[i][1]) {
                case 'f':
                    fprintf(stdout, "Starting synapse service.\n");
                    state = FOREGROUND;
                    break;
                case 'l':
                    // fprintf(stdout, "Collecting logs to %s file\n", argv[i + 1]);
                    strcpy(logpath, argv[i + 1]);
                    break;
                case 'c':
                    // fprintf(stdout, "Reading config file %s\n", argv[i + 1]);
                    strcpy(cfgpath, argv[i + 1]);
                    break;
                case 'n':
                    // fprintf(stdout, "Number of contexts increased to %d\n", atoi(argv[i + 1]));
                    n_contexts = atoi(argv[i + 1]);
                    break;
                default:
                    fprintf(stdout, "Option not recognized\n");
                    exit(EXIT_FAILURE);
                    break;
            }
        }
        // fprintf(stdout, "%s\n", argv[i]);
    }
    freopen(logpath, "a", stderr);
    fprintf(stderr, "Starting reading configuration on %s.\n", cfgpath);
    contexts = (context_t*) calloc(sizeof(context_t), n_contexts);
    load_config(cfgpath, contexts);
    int status = event_loop(argc, argv);
    /*if (state == BACKGROUND) {
        pid_t sid = 0;
        pid_t process_id = 0;
        // Create a fork of this application to start the daemon
        process_id = fork();
        if (process_id < 0) exit(EXIT_FAILURE);

        // Exitting parent process
        if (process_id > 0) {
            fprintf(stderr, "Starting synapse service at %d.\n", process_id);
            exit(EXIT_SUCCESS);
        }
        // Prepare the system to start the daemon
        umask(0);
        sid = setsid();
        if (sid < 0) exit(EXIT_FAILURE);
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }*/
    return 0;
}