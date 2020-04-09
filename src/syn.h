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

#define AV 0
#define HDMI 1
#define LOOP 1
#define VERSION "0.1"
#define MAX_TYPES 2
#define MAX_MODULES 10
#define MAX_BUFFER 128
#define MAX_PATH_SIZE 64
#define DEFAULT_CONFIGURATION_PATH "/opt/efforia/efforia.cfg"
#define DEFAULT_SWITCHER_PATH "/tmp/eos.access"
#define DASHBOARD_PATH "/usr/lib/kodi/kodi.bin"
#define CORONAE_PATH "/opt/efforia/coronae"
#define HDMISTATUS_COMMAND_PATH "/opt/vc/bin/tvservice -s"

typedef struct {
	char type[25];
	char* modules[MAX_MODULES];
	int n_modules;
} Config;

uv_loop_t *loop;
uv_process_t child_req;
uv_process_options_t options;

int64_t counter;

typedef void (*init_plugin_function)();

void cycle();
void idle(uv_idle_t* handle);
void async_start_sensor(uv_work_t *req_dyn);
void async_stop_sensor(uv_work_t *req, int status);

void async_start_process();
void async_end_process(uv_process_t *req, int64_t exit_status, int term_signal);

char** config(const char* filename, int* cnt);
void modules(const char* filename, Config* relations);
void start(const char* module, int max, char** buffer);
void schedule(const char* module);
void background();
void dashboard();

#endif /* SYN_H */
