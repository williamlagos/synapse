#include<dlfcn.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#define MAX_PATH_SIZE 64
#define MAX_BUFFER 128

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

void start(const char* module, int max, char** buffer)
{
	char* error;
	void* handle;
	char path[128];
	void (*function)(int,char**);
	// Prepare the module string path and open library
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

int main(int argc, char** argv)
{
    if(argc < 2) exit(EXIT_FAILURE);
    char* module = argv[1];
    char** buffer;
	int max;
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
	fprintf(stdout,"Started service %s: %d\n",module,service_id);
    return EXIT_SUCCESS;
}
