#ifndef SYN_H
#define SYN_H

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <uv.h>

#define BACKGROUND 0
#define FOREGROUND 1
#define MAX_SENSORS 8
#define SUCCESS_STATUS 0 
#define FAILURE_STATUS 1 
#define DEFAULT_MAX_CONTEXT 8
#define DEFAULT_MAX_PROCESS 8
#define DEFAULT_LOG_PATH "synapse.log"
#define DEFAULT_CONFIGURATION_PATH "default.cfg"
#define DEFAULT_SWITCHER_PATH "/tmp/synapse.access"

typedef struct {
	char name[64];
	char* command;
	char* command_args;
	char* state;
	char* state_args;
	char* sensors[MAX_SENSORS];
	int n_sensors;
} config_t;

typedef struct {
	int status;
	int worker_n;
	char worker_name[64];
	char* worker_args;
} worker_t;

typedef struct {
	config_t config;
	void* data;
	int n_workers;
	uv_work_t* workers;
	uv_process_t main_process;
	uv_process_options_t options;
} context_t;

context_t* contexts;
uv_loop_t* loop;

int64_t counter;

typedef int (*init_sensor_function)(int args, char** buffer);

void idle(uv_idle_t* handle);

void async_start_sensor(uv_work_t *req_dyn);
void async_stop_sensor(uv_work_t *req, int status);
void async_schedule_sensor(context_t* context, int i);

void async_start_process(context_t* context, int i);
void async_end_process(uv_process_t* child_req, int64_t exit_status, int term_signal);

void main_cycle();

#endif /* SYN_H */
