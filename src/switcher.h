#ifndef SWITCHER_H
#define SWITCHER_H

#include "syn.h"
#include <uv_sockets.h>

void start_switcher(uv_loop_t* lp, const char* tcp_host, uint16_t tcp_port);
void stop_switcher(void);

#endif /* SWITCHER_H */
