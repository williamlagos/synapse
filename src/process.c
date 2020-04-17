#include "syn.h"

void async_end_process(uv_process_t* child_req, int64_t exit_status, int term_signal) 
{
    fprintf(stderr, "Process exited with status %d, signal %d\n", (int) exit_status, term_signal);
	child_req->data = (int*) &exit_status; 
}

void async_start_process(context_t* context, int i) 
{
	char* args[3];
	config_t cfg = context->config;
	args[0] = malloc(strlen(cfg.command));
	args[1] = malloc(strlen(cfg.command_args));
    strcpy(args[0], cfg.command);
    strcpy(args[1], cfg.command_args);
    args[2] = NULL;

	uv_process_options_t* options = &(context->options);
    options->exit_cb = async_end_process;
    options->file = context->config.command;
    options->args = args;

    int r;
	uv_process_t* process = &(context->main_process);
    if ((r = uv_spawn(loop, process, options))) {
        fprintf(stderr, "%s\n", uv_strerror(r));
        return;
    } else {
        fprintf(stderr, "Launched process with ID %d\n", process->pid);
    }
}
