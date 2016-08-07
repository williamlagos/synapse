#include<stdio.h>
#include<dlfcn.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>

#define LOOP 1
#define VERSION "0.1"

void config()
{
	FILE *stream;
	char *line = NULL;
	char *key, *value;
	size_t len = 0;
	ssize_t linelen;
	stream = fopen("efforia.cfg", "r");
	if (stream == NULL) exit(EXIT_FAILURE);
	while ((linelen = getline(&line, &len, stream)) != -1) {
		if (strchr(line,'[') != NULL) continue;
		else printf("Line of length %zu :\n", linelen);
		//printf("%s", line);
		key = strtok (line," =");
		printf("Key: %s\n",key);
		value = strtok (NULL, " =");
		printf("Value: %s\n",value);
	}
	free(line);
	fclose(stream);
}

void load(const char* module,const char* func)
{
	char* path;
	char* error;
	void* handle;
	void (*function)();
	// Prepare the module string path and open library
	path = malloc(sizeof(char)*25 + sizeof(module));
	sprintf(path,"modules/lib%s.so.1",module);
	handle = dlopen(path,RTLD_LAZY);
	// Verify if the loading was successful
	if (!handle) {
		fputs(dlerror(), stderr);
		exit(EXIT_FAILURE);
	}
	// Load the specified function in the module
	function = dlsym(handle,func);
	if ((error = dlerror()) != NULL) {
		fputs(error, stderr);
		exit(EXIT_FAILURE);
	}
	// Execute the function, then close the module
	function();
	dlclose(handle);
}

void background()
{
	pid_t sid = 0;
	pid_t process_id = 0;
	// Create a fork of this application to start the daemon
	process_id = fork();
	if (process_id < 0) {
		printf("Process forking failed!\n");
		exit(EXIT_FAILURE);
	}
	// Exitting parent process
	if (process_id > 0) {
		printf("Started Efforia Core %s: %d\n",VERSION,process_id);
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

int main (int argc, char** argv)
{
	config();
	load("hello","helloworld");
	background();
	// Open the file log and start the background service loop
	FILE* f = NULL;
	char info[30] = "Logging some information...\n";
  	f = fopen("efforia.log","w+");
  	while(LOOP) {
    	sleep(1);
    	fprintf(f,info);
    	fflush(f);
  	}
  	fclose(f);
  	return 0;
}
