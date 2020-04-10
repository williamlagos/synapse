#include "syn.h"

void sync_start_sensor(const char* module, int max, char** buffer)
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

void idle(uv_idle_t* handle) {
    counter++;

    if (counter >= 10e6)
        uv_idle_stop(handle);
}

void async_schedule_sensor(context_t* context, int i) {
	// Prepare the module string path and open library
	uv_work_t* w = (uv_work_t*) &(context->workers[i]);
	worker_t* data = (worker_t*) w->data;
    fprintf(stdout,"Opening thread for %s\n", data->worker_name);
    uv_queue_work(loop, &(context->workers[i]), async_start_sensor, async_stop_sensor);
}

// Prepares to run module by dynamic loading
void async_start_sensor(uv_work_t *req_dyn) 
{
    uv_lib_t *lib = (uv_lib_t*) malloc(sizeof(uv_lib_t));
	worker_t* d = (worker_t*) req_dyn->data;

	// Dispatch error if the module can't be loaded:
    if (uv_dlopen(d->worker_name, lib)) {
		fprintf(stderr, "Error: %s\n", uv_dlerror(lib));
		return;
	}

	// Loads function pointer with module, if doesn't, returns error
    init_sensor_function sensor_init;
    if (uv_dlsym(lib, "start", (void **) &sensor_init)) {
		fprintf(stderr, "dlsym error: %s\n", uv_dlerror(lib));
		return;
	}

	// Loads function from module
    // sprintf((char*) req_dyn->data, "%d", sensor_init());
	d->status = (int) sensor_init(0, NULL);
	uv_dlclose(lib);
}

void async_stop_sensor(uv_work_t *req, int status) {
	worker_t* w = (worker_t*) req->data;
    fprintf(stderr, "Done loading %s, status: %d\n", w->worker_name, w->status);
	sensor_event_cycle((uv_req_t*) req);
}