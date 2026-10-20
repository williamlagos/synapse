#include "syn.h"

void async_end_process(uv_process_t* child_req, int64_t exit_status, int term_signal)
{
    fprintf(stderr, "Process exited with status %d, signal %d\n", (int) exit_status, term_signal);
    child_req->data = NULL;
    uv_close((uv_handle_t*) child_req, NULL);
}

void async_start_process(context_t* context, int i)
{
    (void) i;
    config_t cfg = context->config;
    if (cfg.command == NULL) {
        return;
    }

    char* args[3];
    args[0] = cfg.command;
    args[1] = (cfg.command_args && cfg.command_args[0] != '\0') ? cfg.command_args : NULL;
    args[2] = NULL;

    uv_process_options_t* options = &(context->options);
    memset(options, 0, sizeof(uv_process_options_t));
    options->exit_cb = async_end_process;
    options->file = cfg.command;
    options->args = args;

    uv_process_t* process = &(context->main_process);
    int r = uv_spawn(loop, process, options);
    if (r != 0) {
        fprintf(stderr, "Spawn error (%s): %s\n", cfg.command, uv_strerror(r));
        return;
    }
    fprintf(stderr, "Launched process with ID %d\n", process->pid);
}
