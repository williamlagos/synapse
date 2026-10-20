#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define DEFAULT_COMMAND_PATH "cat worked.file"
#define VERSION "0.1"

pid_t app_id = 0;

void start_plugin(const char* module, int max, char** buffer)
{
	char* error;
	void* handle;
	char path[128];
	void (*function)(int,char**);
	// Prepare the module string path and open library
	sprintf(path, "./%s.so", module);
	handle = dlopen(path, RTLD_LAZY);
	// Verify if the loading was successful
	if (!handle) {
		fputs(dlerror(), stderr);
		exit(EXIT_FAILURE);
	}
	// Load the specified function in the module
	function = dlsym(handle, "start");
	if ((error = dlerror()) != NULL) {
		fputs(error, stderr);
		exit(EXIT_FAILURE);
	}
	// Execute the function, then close the module
	function(max,buffer);
	dlclose(handle);
}

void start_process(char* process, char* process_args)
{
	pid_t sid = 0;
	pid_t process_id = 0;
	// Create a fork of this application to start the daemon
	process_id = fork();
	if (process_id < 0) exit(EXIT_FAILURE);

	// Exitting parent process
	if (process_id > 0) {
		fprintf(stdout,"Started impulse %s: %d\n", VERSION, process_id);
		exit(EXIT_SUCCESS);
	}
	// Prepare the system to start the daemon
	umask(0);
	sid = setsid();
	if (sid < 0) exit(EXIT_FAILURE);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

int start(int max, char** buffer)
{
    char cmd[256] = "";
    if (max < 2 && max > 0) {
        strcpy(cmd, DEFAULT_COMMAND_PATH);
    } else if (max == 0) {
        strcpy(cmd, buffer[0]);
    } else {
        for (int i = 1; i < max; i++) {
            strcat(cmd, buffer[i]);
            strcat(cmd, " ");
        }
    }

    FILE *f = popen(cmd, "r");
    if(f == NULL){
        fprintf(stderr, "error: failed to run command\n");
        return EXIT_FAILURE;
    }

    int pstatus = pclose(f);
    if (pstatus == -1) return EXIT_FAILURE;
    int status = WEXITSTATUS(pstatus);
    return status;
}

int main(int argc, char** argv)
{
    return start(argc, argv);
}
