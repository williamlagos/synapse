#include "syn.h"

pid_t app_id = 0;

// Read configuration file and return lines buffer to manipulate
/*char** config(const char* filename, int* cnt)
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
}*/

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

void schedule(const char* module)
{
	char executable[MAX_BUFFER];
	sprintf(executable,"%s %s",CORONAE_PATH,module);
	system(executable);
}

void background()
{
	pid_t sid = 0;
	pid_t process_id = 0;
	// Create a fork of this application to start the daemon
	process_id = fork();
	if (process_id < 0) exit(EXIT_FAILURE);

	// Exitting parent process
	if (process_id > 0) {
		fprintf(stdout,"Started Efforia Core %s: %d\n", VERSION, process_id);
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

void dashboard()
{
	//char executable[MAX_BUFFER];
	//sprintf(executable,"%s &",DASHBOARD_PATH);
	app_id = fork();
	if(app_id == 0){
		fprintf(stdout,"Started Dashboard: %d",app_id);
		execv(DASHBOARD_PATH,NULL);
	}
	//sprintf(executable,"%s event",CORONAE_PATH);
	//system(executable);
}

void async_end_process(uv_process_t *req, int64_t exit_status, int term_signal) 
{
    fprintf(stderr, "Process exited with status %d, signal %d\n", (int) exit_status, term_signal);
    uv_close((uv_handle_t*) req, NULL);
}

void async_start_process() 
{
	char* args[3];
    args[0] = "mkdir";
    args[1] = "test-dir";
    args[2] = NULL;

    options.exit_cb = async_end_process;
    options.file = "mkdir";
    options.args = args;

    int r;
    if ((r = uv_spawn(loop, &child_req, &options))) {
        fprintf(stderr, "%s\n", uv_strerror(r));
        return;
    } else {
        fprintf(stderr, "Launched process with ID %d\n", child_req.pid);
    }
}

int process(int argc, char** argv)
{
	int count;
	int video = AV;
	char app[64], app_type[64];
	Config apps, backends, *r;
	r = (Config*) malloc(sizeof(Config) * MAX_TYPES);
	modules(DEFAULT_CONFIGURATION_PATH, r);
	apps = (strstr(r[1].type, "app") != NULL) ? r[1] : r[0];
	backends = (strstr(r[0].type, "backend") != NULL) ? r[0] : r[1];

	if(apps.n_modules == 2){
		strcpy(app,apps.modules[0]);
		strcpy(app_type,apps.modules[1]);
	}
	int last = backends.n_modules - 1;
	backends.modules[last] = NULL;
	backends.n_modules--;
	for (count = 0; count < backends.n_modules; count++){
		char* backend = backends.modules[count];
		if(strstr(backend,"stream")) video = HDMI;
		schedule(backend);
	}

	free(r);

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
  	return 0;
}
