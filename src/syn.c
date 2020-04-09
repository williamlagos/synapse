#include "syn.h"

#define FIB_UNTIL 10

// uv_work_t dyn_req[4];
uv_work_t req[FIB_UNTIL];

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
void load_config(const char* filename, config_t* relations, int n_relations)
{
	int count, l, max;
	char *key, *v, *value;
	char **lines = load_buffer(filename, &max);
	if (lines == NULL) exit(EXIT_FAILURE);
    config_t* r = &relations[0];
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
            if (r->command != NULL && r->name != NULL) r++;
        }
	}
}

void start_fib(uv_work_t *req) {
    int n = *(int *) req->data;
    if (random() % 2)
        uv_sleep(1);
    else
        uv_sleep(3);
    // long fib = fib_(n);
    int i, fib;
    int a = 0;
    int b = 1;
    for(i = 0; i < n; i++) {
        fib = a + b;
        a = b;
        b = fib;
    }
    fprintf(stderr, "%dth fibonacci is %lu\n", n, fib);
}

void stop_fib(uv_work_t *req, int status) {
    fprintf(stderr, "Done calculating %dth fibonacci\n", *(int *) req->data);
}

void signal_handler(uv_signal_t *req, int signum)
{
    printf("Signal received!\n");
    int i;
    // uv_cancel((uv_req_t*) dyn_req);
    for (i = 0; i < FIB_UNTIL; i++) {
        uv_cancel((uv_req_t*) &req[i]);
    }
    uv_signal_stop(req);
}

void event_cycle(uv_handle_t* handle) {
    if (handle->type == UV_PROCESS) {
        if (atoi(handle->data) == 20) {
            fprintf(stdout, "Fork Hello!\n");
        } else {
            fprintf(stdout, "Fork No.\n");
        }
    } else if (handle->type == UV_ASYNC) {
        if (atoi(handle->data) == 20) {
		    fprintf(stdout, "Thread Hello!\n");
        } else {
            fprintf(stdout, "Thread No.\n");
        }
    }
    
    // uv_close((uv_handle_t*) handle, NULL);
    // free(handle);
}

int event_loop(int argc, char** argv) {
    loop = uv_default_loop();

    /*uv_timer_t timer_req;

    uv_timer_init(loop, &timer_req);
    uv_timer_start(&timer_req, cycle, 5000, 2000);*/

    int data[FIB_UNTIL];
    int i;
    for (i = 0; i < FIB_UNTIL; i++) {
        data[i] = i;
        req[i].data = (void *) &data[i];
        uv_queue_work(loop, &req[i], start_fib, stop_fib);
    }

    /*uv_signal_t sig;
    uv_signal_init(loop, &sig);
    uv_signal_start(&sig, signal_handler, SIGINT);*/

    uv_idle_t idler;
    counter = 0;
    uv_idle_init(loop, &idler);
    uv_idle_start(&idler, idle);

    printf("Idling...\n");

    config_t processes, sensors, *r;
	r = (config_t*) malloc(sizeof(config_t) * MAX_CONFIGS);
	load_config(DEFAULT_CONFIGURATION_PATH, r, MAX_CONFIGS);

	for (int i = 0; i < MAX_CONFIGS; i++) {
        fprintf(stdout, "Starting configuration for %s...\n", r[i].name);
        fprintf(stdout, "Command: %s %s starting\n", r[i].command, r[i].command_args);
        child_req[i].data = (void*) "10";
        async_start_process(&child_req[i], r[i].command, r[i].command_args);
        for(int s = 0; s < r[i].n_sensors; s++) {
            uv_work_t* dyn_req = malloc(sizeof(uv_work_t));
            fprintf(stdout, "Sensor: %s activated\n", r[i].sensors[s]);
            async_schedule_sensor(dyn_req, r[i].sensors[s]);

            /*if (argc == 1) {
            fprintf(stderr, "Usage: %s [plugin1] [plugin2] ...\n", argv[0]);
                // return 0;
            } else {
                
                char* data = malloc(strlen(argv[1]));
                // char data[20];
                strcpy(data, argv[1]);*/
                
                // fprintf(stdout,"Opening thread for %s\n", data);
                
            // }
        }
	}



    return uv_run(loop, UV_RUN_DEFAULT);
}

int main(int argc, char** argv) {
    /* Sensor main block:
    if(argc < 2) exit(EXIT_FAILURE);
    char* module = argv[1];
    char** buffer;
	int max;
	char path[MAX_PATH_SIZE];
	sprintf(path, "./%s.cfg", module);
	buffer = config(path, &max);
	pid_t service_id = fork();
	if(service_id == 0){
		// Start backend with config
		if(buffer != NULL) start(module,max,buffer);
		// Without configuration
		else start(module,0,NULL);
	}
	fprintf(stdout,"Started service %s: %d\n",module,service_id);
    return EXIT_SUCCESS; */
 
    /* Process main block:
    // int count;
	// int video = AV;
	// char app[64], app_type[64];
	if(apps.n_modules == 2){
		strcpy(app,apps.modules[0]);
		strcpy(app_type,apps.modules[1]);
	}

	free(r);

    if(strstr(backend,"stream")) video = HDMI;
	if(video == HDMI){
		char path[128];
		FILE *f = popen(HDMISTATUS_COMMAND_PATH, "r");
		fgets(path, sizeof(path)-1, f);
		if(strstr(path,"NTSC") || strstr(path,"PAL")){
			fprintf(stdout,"Running in headless mode.\n");
			exit(EXIT_SUCCESS);
		}
	}

	if(strcmp(app,"dashboard") == 0) dashboard();
	else {
			char executable[128];
			sprintf(executable,"/opt/efforia/applications/%s",app);
			system(executable);
			system("shutdown -h now");
	}

	// Open the file log and start the background service loop
	//background();
	char executable[MAX_BUFFER];
  	while(LOOP) {
    	sleep(1);
		if(access(DEFAULT_SWITCHER_PATH,F_OK) != -1) {
			kill(app_id,SIGTERM);
			FILE *f = fopen(DEFAULT_SWITCHER_PATH,"r");
			fgets(executable,sizeof(executable),f);
			fclose(f);
			remove(DEFAULT_SWITCHER_PATH);
			system(executable);
			dashboard();
		}
  	}
  	return 0;*/

    return event_loop(argc, argv);

    // return EXIT_SUCCESS;
}