#ifndef TIMER_H
#define TIMER_H

#include "Queue.h"
#include "Http_request.h"

#define TINY_TIMER_INFINITE -1
#define TIMEOUT_DEFAULT 500     /* ms */

typedef int (*timer_handler_pt)(http_request_t *rq);

typedef struct timer_node_s{
    size_t key;
    int deleted;//如果客户端关闭了，置为1
    timer_handler_pt handler;
    http_request_t *rq;
}tiny_timer_node;

int timer_init();
int find_timer();
void handle_expire_timers();

extern pq_t timer;
extern size_t current_msec;

void add_timer(http_request_t *rq, size_t timeout, timer_handler_pt handler);
void del_timer(http_request_t *rq);

#endif

