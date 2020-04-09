#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <unistd.h>
#include <uv.h>

#include "syn.h"

#define FIB_UNTIL 10

uv_loop_t *loop;
uv_process_t child_req;
uv_process_options_t options;
uv_work_t* dyn_req;
uv_work_t req[FIB_UNTIL];

void on_exit1(uv_process_t *req, int64_t exit_status, int term_signal) {
    fprintf(stderr, "Process exited with status %d, signal %d\n", (int) exit_status, term_signal);
    uv_close((uv_handle_t*) req, NULL);
}

long fib_(int n) {
    int i, aux;
    int a = 0;
    int b = 1;
    for(i = 0; i < n; i++) {
        aux = a + b;
        a = b;
        b = aux;
    }
}

void fib(uv_work_t *req) {
    int n = *(int *) req->data;
    if (random() % 2)
        uv_sleep(1);
    else
        uv_sleep(3);
    long fib = fib_(n);
    fprintf(stderr, "%dth fibonacci is %lu\n", n, fib);
}

void after_fib(uv_work_t *req, int status) {
    fprintf(stderr, "Done calculating %dth fibonacci\n", *(int *) req->data);
}

typedef void (*init_plugin_function)();

void dyn(uv_work_t *req_dyn) {
    char* module = malloc(strlen(req_dyn->data));
    strcpy(module, req_dyn->data);
    uv_lib_t *lib = (uv_lib_t*) malloc(sizeof(uv_lib_t));

    // while (--argc) {
    fprintf(stdout,"Opened thread for %s\n", module);
    fprintf(stderr, "Loading %s\n", module);
    if (uv_dlopen(module, lib)) {
        fprintf(stderr, "Error: %s\n", uv_dlerror(lib));
        // continue;
    }

    init_plugin_function init_plugin;
    if (uv_dlsym(lib, "start", (void **) &init_plugin)) {
        fprintf(stderr, "dlsym error: %s\n", uv_dlerror(lib));
        // continue;
    }

    init_plugin();
    uv_dlclose(lib);
    free(lib);
    // }
}

void after_dyn(uv_work_t *req, int status) {
    fprintf(stderr, "Done loading %s\n", (char *) req->data);
    free(req);
}

void signal_handler(uv_signal_t *req, int signum)
{
    printf("Signal received!\n");
    int i;
    uv_cancel((uv_req_t*) dyn_req);
    for (i = 0; i < FIB_UNTIL; i++) {
        uv_cancel((uv_req_t*) &req[i]);
    }
    uv_signal_stop(req);
}

int64_t counter = 0;

void callback() {
    printf("Hello World!");
}

void wait_for_a_while(uv_idle_t* handle) {
    counter++;

    if (counter >= 10e6)
        uv_idle_stop(handle);
}

int main(int argc, char** argv) {
    loop = uv_default_loop();


    /*uv_timer_t timer_req;

    uv_timer_init(loop, &timer_req);
    uv_timer_start(&timer_req, callback, 5000, 2000);*/


    /*if (argc == 1) {
        fprintf(stderr, "Usage: %s [plugin1] [plugin2] ...\n", argv[0]);
        // return 0;
    } else {
        dyn_req = malloc(sizeof(uv_work_t));
        char* data = malloc(strlen(argv[1]));
        // char data[20];
        strcpy(data, argv[1]);
        dyn_req->data = (void*) data;
        // fprintf(stdout,"Opening thread for %s\n", data);
        fprintf(stdout,"Opening thread for %s\n", (char*) dyn_req->data);
    
        uv_queue_work(loop, &dyn_req[0], dyn, after_dyn);
        
    }*/


    int data[FIB_UNTIL];
    int i;
    for (i = 0; i < FIB_UNTIL; i++) {
        data[i] = i;
        req[i].data = (void *) &data[i];
        uv_queue_work(loop, &req[i], fib, after_fib);
    }


    char* args[3];
    args[0] = "mkdir";
    args[1] = "test-dir";
    args[2] = NULL;

    options.exit_cb = on_exit1;
    options.file = "mkdir";
    options.args = args;

    int r;
    if ((r = uv_spawn(loop, &child_req, &options))) {
        fprintf(stderr, "%s\n", uv_strerror(r));
        return 1;
    } else {
        fprintf(stderr, "Launched process with ID %d\n", child_req.pid);
    }


    /*uv_signal_t sig;
    uv_signal_init(loop, &sig);
    uv_signal_start(&sig, signal_handler, SIGINT);*/

    uv_idle_t idler;

    uv_idle_init(loop, &idler);
    uv_idle_start(&idler, wait_for_a_while);

    printf("Idling...\n");

    return uv_run(loop, UV_RUN_DEFAULT);;
}