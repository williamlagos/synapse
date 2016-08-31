#include<stdio.h>
#include<dlfcn.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<sys/stat.h>
#include<sys/types.h>
#include "efforia.h"

pid_t app_id = 0;

// Read configuration file and return lines buffer to manipulate
char** config(const char* filename, int* cnt)
{
	int count = (*cnt);
	char **lines;
	char buffer[MAX_BUFFER];
	FILE *f = fopen(filename, "r");
	if (f == NULL) return NULL;
	lines = (char**) calloc(sizeof(char*), MAX_BUFFER);
	for (count = 0; fgets(buffer, sizeof(buffer), f); count++) {
		lines[count] = (char*) malloc(sizeof(buffer));
		strcpy(lines[count], buffer);
	}
	fclose(f);
	(*cnt) = count;
	return lines;
}

// Read main configuration file and prepares list of modules to be used
void modules(const char* filename, Config* relations)
{
	int count, lin, max;
	char *key, *val, *value;
	char **lines = config(filename, &max);
	if(lines == NULL) exit(EXIT_FAILURE);
	for (lin = 0; lin < max; lin++) {
		if (strchr(lines[lin], '[') != NULL) continue;
		key = strtok(lines[lin], " =");
		value = strtok(NULL, " =");
		val = strtok(value, ", ");
		count = 0;
		while (val != NULL) {
			strcpy(relations->type, key);
			relations->modules[count] = (char*) malloc(sizeof(val));
			strcpy(relations->modules[count], val);
			val = strtok(NULL, ", ");
			count++;
		}
		relations->n_modules = count;
		relations++;
	}
}

void start(const char* module, int max, char** buffer)
{
	char* error;
	void* handle;
	char path[128];
	void (*function)(int,char**);
	// Prepare the module string path and open library
	fprintf(stdout, "/opt/efforia/services/%s.so\n", module);
	sprintf(path, "/opt/efforia/services/%s.so", module);
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

void schedule(const char* module)
{
	char** buffer;
	int count, max;
	char path[MAX_PATH_SIZE];
	sprintf(path, "/opt/efforia/services/%s.cfg", module);
	buffer = config(path, &max);
	pid_t service_id = fork();
	if(service_id == 0){
		// Start backend with config
		if(buffer != NULL) start(module,max,buffer);
		// Without configuration
		else start(module,0,NULL);
	}
}

void background()
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
}

int dashboard()
{
	app_id = fork();
	if (app_id == 0)
		execv(DASHBOARD_PATH,NULL);
	return app_id;
}

int main (int argc, char** argv)
{
	int count;
	char app[64], app_type[64];
	Config apps, backends, *r;
	r = (Config*) malloc(sizeof(Config) * MAX_TYPES);
	modules(DEFAULT_CONFIGURATION_PATH, r);
	apps = (strstr(r[1].type, "app") != NULL) ? r[1] : r[0];
	backends = (strstr(r[0].type, "backend") != NULL) ? r[0] : r[1];

	if(apps.n_modules == 2){
		strcpy(app,apps.modules[0]);
		strcpy(app_type,apps.modules[1]);
	}
	int last = backends.n_modules - 1;
	backends.modules[last] = NULL;
	backends.n_modules--;
	for (count = 0; count < backends.n_modules; count++)
		schedule(backends.modules[count]);
	free(r);

	char path[128];
	FILE *f = popen(HDMISTATUS_COMMAND_PATH, "r");
    fgets(path, sizeof(path)-1, f);
    if(strstr(path,"NTSC") || strstr(path,"PAL")){
		fprintf(stdout,"Running in headless mode.\n");
        exit(EXIT_SUCCESS);
    }
	dashboard();

	// Open the file log and start the background service loop
	background();
	char executable[MAX_BUFFER];
  	while(LOOP) {
    	sleep(1);
		if(access(DEFAULT_SWITCHER_PATH,F_OK) != -1) {
			kill(app_id,SIGTERM);
			f = fopen(DEFAULT_SWITCHER_PATH,"r");
			fgets(executable,sizeof(executable),f);
			fclose(f);
			remove(DEFAULT_SWITCHER_PATH);
			system(executable);
			dashboard();
		}
  	}
  	return 0;
}
