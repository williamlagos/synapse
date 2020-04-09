#include "syn.h"

pid_t app_id = 0;

void sync_start_process(char* process, char* process_args)
{
	pid_t sid = 0;
	pid_t process_id = 0;
	// Create a fork of this application to start the daemon
	process_id = fork();
	if (process_id < 0) exit(EXIT_FAILURE);

	// Exitting parent process
	if (process_id > 0) {
		fprintf(stdout,"Started Efforia Core %s: %d\n", VERSION, process_id);
		exit(EXIT_SUCCESS);
	}
	// Prepare the system to start the daemon
	umask(0);
	sid = setsid();
	if (sid < 0) exit(EXIT_FAILURE);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/*void dashboard() {
	//char executable[MAX_BUFFER];
	//sprintf(executable,"%s &",DASHBOARD_PATH);
	app_id = fork();
	if(app_id == 0){
		fprintf(stdout,"Started Dashboard: %d",app_id);
		execv(DASHBOARD_PATH,NULL);
	}
	//sprintf(executable,"%s event",CORONAE_PATH);
	//system(executable);
	}*/

	/*void sync_start_process(const char* module)
	{
		char executable[MAX_BUFFER];
		sprintf(executable,"%s %s",CORONAE_PATH,module);
		system(executable);
	}*/
}

void async_end_process(uv_process_t* child_req, int64_t exit_status, int term_signal) 
{
    fprintf(stderr, "Process exited with status %d, signal %d\n", (int) exit_status, term_signal);
	event_cycle((uv_handle_t*) child_req);
}

void async_start_process(uv_process_t* child_req, 
						 char* process, 
						 char* process_args) 
{
	char* args[3];
	args[0] = malloc(strlen(process));
	args[1] = malloc(strlen(process_args));
    strcpy(args[0], process);
    strcpy(args[1], process_args);
    args[2] = NULL;

    options.exit_cb = async_end_process;
    options.file = process;
    options.args = args;

    int r;
    if ((r = uv_spawn(loop, child_req, &options))) {
        fprintf(stderr, "%s\n", uv_strerror(r));
        return;
    } else {
        fprintf(stderr, "Launched process with ID %d\n", child_req->pid);
    }
}
