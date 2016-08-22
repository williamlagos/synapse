#include<stdio.h>
#include<dlfcn.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>

#define LOOP 1
#define VERSION "0.1"
#define MAX_TYPES 2
#define MAX_MODULES 10
#define MAX_BUFFER 128
#define MAX_PATH_SIZE 64
#define DEFAULT_CONFIGURATION_PATH "efforia.cfg"

typedef struct {
	char type[25];
	char* modules[MAX_MODULES];
	int n_modules;
} Config;

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

void start(const char* module, const char* func)
{
	char* path;
	char* error;
	void* handle;
	void (*function)();
	// Prepare the module string path and open library
	path = malloc(sizeof(char)*25 + sizeof(module));
	sprintf(path, "modules/lib%s.so.1", module);
	handle = dlopen(path, RTLD_LAZY);
	// Verify if the loading was successful
	if (!handle) {
		fputs(dlerror(), stderr);
		exit(EXIT_FAILURE);
	}
	// Load the specified function in the module
	function = dlsym(handle, func);
	if ((error = dlerror()) != NULL) {
		fputs(error, stderr);
		exit(EXIT_FAILURE);
	}
	// Execute the function, then close the module
	function();
	dlclose(handle);
}

void schedule(const char* type, const char* module) {
	char** buffer;
	int count, max;
	char path[MAX_PATH_SIZE];
	sprintf(path, "config/%s.json", module);
	buffer = config(path, &max);
	if (buffer != NULL) {
		printf("Config of %s %s:\n",module,type);
		for (count = 0; count < max; count++) {
			printf("%s", buffer[count]);
		}
	} else {
		printf("%s %s without configuration\n",module,type);
	}
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
		printf("Started Efforia Core %s: %d\n", VERSION, process_id);
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
	int count;
	Config apps, backends, *r;
	r = (Config*) malloc(sizeof(Config) * MAX_TYPES);
	modules(DEFAULT_CONFIGURATION_PATH, r);
	apps = (strstr(r[1].type, "app") != NULL) ? r[1] : r[0];
	backends = (strstr(r[0].type, "backend") != NULL) ? r[0] : r[1];

	for (count = 0; count < apps.n_modules; count++)
		schedule(apps.type, apps.modules[count]);
	for (count = 0; count < backends.n_modules; count++)
		schedule(backends.type, backends.modules[count]);
	free(r);
	load("hello", "helloworld");
	//background();
	// Open the file log and start the background service loop
	FILE* f = NULL;
	char info[30] = "Logging some information...\n";
  	f = fopen("efforia.log", "w+");
  	/*while(LOOP) {
    	sleep(1);
    	fprintf(f, info);
    	fflush(f);
  	}*/
  	fclose(f);
  	return 0;
}
