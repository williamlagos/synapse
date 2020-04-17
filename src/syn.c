// #include "syn.h"
#include "utils.h"

// TODO: Make logics about finishing sensor cycle
void sensor_event_cycle(uv_req_t* req) {
    if (req->type == UV_WORK) {
        worker_t* w = (worker_t*) req->data;
        if (w->status == 20) {
		    fprintf(stdout, "Thread Hello!\n");
        } else {
            fprintf(stdout, "Thread No.\n");
        }
    }
}

// TODO: Make logics about finishing process cycle
void process_event_cycle(uv_handle_t* handle) {
    if (handle->type == UV_PROCESS) {
        if (atoi(handle->data) == 0) {
            fprintf(stdout, "Fork Hello!\n");
        } else {
            fprintf(stdout, "Fork No.\n");
        }
    }
}

void main_cycle() {
	for (int i = 0; i < MAX_CONFIGS; i++) {
        context_t* context = &contexts[i];
        config_t* r = &(context->config);
        fprintf(stdout, "Starting configuration for %s...\n", r->name);
        context->n_workers = r->n_sensors;
        if (context->workers == NULL) {
            context->workers = calloc(sizeof(uv_work_t), context->n_workers);
        }
        for (int s = 0; s < r->n_sensors; s++) {
            uv_work_t* worker_handle = (uv_work_t*) &(context->workers[s]);
            if ((worker_t*) worker_handle->data == NULL) {
                worker_t* worker = (worker_t*) malloc(sizeof(worker_t));
                worker->status = -10;
	            worker->worker_n = s;
                // Verify if worker has arguments to pass
                if (r->state_args == NULL || r->state == NULL) {
                    worker->worker_args = (char*) malloc(sizeof(char));
                    strcpy(worker->worker_args, " ");
                } else {
                    worker->worker_args = (char*) malloc(sizeof(r->state) + sizeof(r->state_args));
                    sprintf(worker->worker_args, "%s %s", r->state, r->state_args);
                }
	            strcpy(worker->worker_name, context->config.sensors[s]);
	            context->workers[s].data = (void*) worker;     
            } else {
                int status = ((worker_t*) worker_handle->data)->status;
                fprintf(stdout, "Status worker: %d\n", status);
                // TODO: Make logics about conditional sensor / process cycle
                if (status == SUCCESS_STATUS) {
                    fprintf(stdout, "Condition met: command: %s %s starting\n", r->command, r->command_args);
                    async_start_process(context, i);
                } else if (status == FAILURE_STATUS) {
                    fprintf(stderr, "Condition not met: command: %s %s not starting\n", r->command, r->command_args);
                }
            }
            fprintf(stdout, "Sensor: %s activated\n", r->sensors[s]);
            async_schedule_sensor(context, s);
        }
	}

}

int event_loop(int argc, char** argv) {
    loop = uv_default_loop();

    uv_signal_t sig;
    uv_signal_init(loop, &sig);
    uv_signal_start(&sig, signal_handler, SIGINT);

    fibonacci_cycle(loop);

    uv_idle_t idler;
    counter = 0;
    uv_idle_init(loop, &idler);
    uv_idle_start(&idler, idle);

    uv_timer_t timer_req;
    uv_timer_init(loop, &timer_req);
    uv_timer_start(&timer_req, main_cycle, 5000, 2000);

    return uv_run(loop, UV_RUN_DEFAULT);
}

int main(int argc, char** argv) {
    contexts = (context_t*) calloc(sizeof(context_t), MAX_CONFIGS);
    // config_t* c = (config_t*) malloc(sizeof(config_t) * MAX_CONFIGS);
	load_config(DEFAULT_CONFIGURATION_PATH, contexts);/*, MAX_CONFIGS);*/
 
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
}