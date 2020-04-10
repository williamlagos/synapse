#ifndef UTILS_H
#define UTILS_H

#include "syn.h"

#define FIB_UNTIL 10

uv_work_t req[FIB_UNTIL];

void signal_handler(uv_signal_t *req, int signum);
char** load_buffer(const char* filename, int* cnt);
void load_config(const char* filename, context_t* contexts);
void fibonacci_cycle(uv_loop_t* l);

#endif /* UTILS_H */