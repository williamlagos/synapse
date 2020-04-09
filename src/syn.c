#include "syn.h"

#define FIB_UNTIL 10

uv_work_t* dyn_req;
uv_work_t req[FIB_UNTIL];

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
    uv_cancel((uv_req_t*) dyn_req);
    for (i = 0; i < FIB_UNTIL; i++) {
        uv_cancel((uv_req_t*) &req[i]);
    }
    uv_signal_stop(req);
}

int main(int argc, char** argv) {
    loop = uv_default_loop();

    /*uv_timer_t timer_req;

    uv_timer_init(loop, &timer_req);
    uv_timer_start(&timer_req, cycle, 5000, 2000);*/

    if (argc == 1) {
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
    
        uv_queue_work(loop, &dyn_req[0], async_start_sensor, async_stop_sensor);
        
    }

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

    return uv_run(loop, UV_RUN_DEFAULT);;
}