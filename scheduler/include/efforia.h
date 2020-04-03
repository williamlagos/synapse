#ifndef EFFORIA_H
#define EFFORIA_H

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

char** config(const char* filename, int* cnt);
void modules(const char* filename, Config* relations);
void start(const char* module, int max, char** buffer);
void schedule(const char* module);
void background();
void dashboard();

#endif
