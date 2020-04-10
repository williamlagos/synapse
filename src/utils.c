#include "utils.h"

// Read file and return lines buffer to manipulate
char** load_buffer(const char* filename, int* cnt)
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
void load_config(const char* filename, context_t* contexts)
{
	int count, l, max;
	char *key, *v, *value;
	char **lines = load_buffer(filename, &max);
	if (lines == NULL) exit(EXIT_FAILURE);
    config_t* r = &(contexts->config);
	for (l = 0; l < max; l++) {
        // Checks for the name of the section then stores
		if (strchr(lines[l], '[') != NULL) {
            char* section = ++lines[l];
            section[strlen(lines[l]) - strlen(strchr(lines[l], ']'))] = 0;
            strcpy(r->name, section);
            continue;
        }

        // Gets key and value from this line
		key = strtok(lines[l], "=");
		value = strtok(NULL, "=\n");

        // Checks if the key is for command then stores
        if (strcmp(key, "command") == 0) {
            r->command = (char*) malloc(strlen(value));
            strcpy(r->command, strtok(value, " "));
            size_t command_size = strlen(r->command);
            r->command_args = (char*) malloc(strlen(value) - command_size);
            strcpy(r->command_args, strtok(NULL, " "));
            while((v = strtok(NULL, " ")) != NULL) sprintf(r->command_args, "%s %s", r->command_args, v);
            continue;
        }

        // Checks if the key is for command then stores
        if (strcmp(key, "sensors") == 0) {
            count = 0;
            v = strtok(value, ",");
            while (v != NULL) {
                r->sensors[count] = (char*) malloc(sizeof(v));
                strcpy(r->sensors[count], v);
                v = strtok(NULL, ",");
                count++;
            }   
            r->n_sensors = count;
            // Check if the configuration was already read, then go to another
            if (r->command != NULL && r->name != NULL) r = &(++contexts)->config;
        }
	}
}

void start_fib(uv_work_t *req) {
    int n = *(int *) req->data;
    if (random() % 2) uv_sleep(1);
    else uv_sleep(3);
    int fib = 0;
    int a = 0;
    int b = 1;
    for(int i = 0; i < n; i++) {
        fib = a + b;
        a = b;
        b = fib;
    }
    fprintf(stderr, "%dth fibonacci is %lu\n", n, fib);
}

void stop_fib(uv_work_t *req, int status) {
    fprintf(stderr, "Done calculating %dth fibonacci\n", *(int *) req->data);
}

// Signal handling basics for dummy cycle
void signal_handler(uv_signal_t *req, int signum)
{
    printf("Signal received!\n");
    int i;
    for (i = 0; i < FIB_UNTIL; i++) {
        uv_cancel((uv_req_t*) &req[i]);
    }
    uv_signal_stop(req);
}

// Dummy fibonacci sequence cycle to test thread workers
void fibonacci_cycle(uv_loop_t* l) {
    int* data = calloc(sizeof(int), FIB_UNTIL);
    for (int i = 0; i < FIB_UNTIL; i++) {
        data[i] = i;
        req[i].data = (void *) &data[i];
        uv_queue_work(l, &req[i], start_fib, stop_fib);
    }
}