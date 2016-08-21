#include<stdio.h>
#include<dlfcn.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>

#define APP 1
#define LOOP 1
#define BACKEND 0
#define VERSION "0.1"
#define MAX_TYPES 2
#define MAX_MODULES 5
#define CONFIG_FILE "efforia.cfg"

typedef struct {
	char type[25];
	char* modules[MAX_MODULES];
} Config;

void config(const char* filename, Config* relations)
{
	int count;
	FILE *stream;
	char *line = NULL;
	char *key, *value, *val;
	size_t len = 0;
	ssize_t linelen;
	stream = fopen(filename, "r");
	if (stream == NULL) exit(EXIT_FAILURE);
	while ((linelen = getline(&line, &len, stream)) != -1) {
		if (strchr(line,'[') != NULL) continue;
		key = strtok(line," =");
		value = strtok(NULL," =");
		val = strtok(value,",");
		count = 0;
		while (val != NULL) {
			strcpy(relations->type,key);
			relations->modules[count] = (char*) malloc(sizeof(val));
			strcpy(relations->modules[count],val);
			val = strtok(NULL,",");
			count++;
		}
		relations++;
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
	int count;
	Config* relations;
	Config apps, backends;
	relations = (Config*) malloc(sizeof(Config) * MAX_TYPES);
	config(CONFIG_FILE,relations);
	backends = relations[BACKEND];
	apps = relations[APP];
	free(relations);
	printf("List of %ss:\n",apps.type);
	for (count = 0; count < MAX_MODULES; count++) {
		printf("%s\n",apps.modules[count]);
	}
	printf("List of %ss:\n",backends.type);
	for (count = 0; count < MAX_MODULES; count++) {
		printf("%s\n",backends.modules[count]);
	}
	load("hello","helloworld");
	//background();
	// Open the file log and start the background service loop
	FILE* f = NULL;
	char info[30] = "Logging some information...\n";
  	f = fopen("efforia.log","w+");
  	/*while(LOOP) {
    	sleep(1);
    	fprintf(f,info);
    	fflush(f);
  	}*/
  	fclose(f);
  	return 0;
}
