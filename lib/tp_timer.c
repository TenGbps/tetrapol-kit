#define LOG_PREFIX "timer"
#include "tetrapol/log.h"
#include "tetrapol/tp_timer.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    timer_callback_t func;
    void *ptr;
} callback_t;

struct tp_timer_priv_t {
    int ncallbacks;
    callback_t *callbacks;
    time_evt_t te;
};

tp_timer_t *tp_timer_create(void)
{
    tp_timer_t *timer = calloc(1, sizeof(tp_timer_t));
    if (!timer) {
        return NULL;
    }

    return timer;
}

void tp_timer_destroy(tp_timer_t *timer)
{
    if (!timer) {
        return;
    }
    free(timer->callbacks);
    free(timer);
}

void tp_timer_tick(tp_timer_t *timer, bool rx_glitch, int usec)
{
    timer->te.tv.tv_usec += usec;
    timer->te.tv.tv_sec += timer->te.tv.tv_usec / 1000000;
    timer->te.tv.tv_usec %= 1000000;
    timer->te.rx_glitch = rx_glitch;

    for (int i = 0; i < timer->ncallbacks; ++i) {
        timer->callbacks[i].func(&timer->te, timer->callbacks[i].ptr);
    }
}

bool tp_timer_register(tp_timer_t *timer, timer_callback_t timer_func, void *ptr)
{
    // check for double-registration
    for (int i = 0; i < timer->ncallbacks; ++i) {
        if (timer->callbacks[i].func == timer_func &&
                timer->callbacks[i].ptr == ptr) {
            LOG(WTF, "double registration of callback");
            return false;
        }
    }

    ++timer->ncallbacks;
    callback_t *p = realloc(timer->callbacks, sizeof(callback_t[timer->ncallbacks]));
    if (!p) {
        LOG(ERR, "ERR OOM");
        return false;
    }
    timer->callbacks = p;
    timer->callbacks[timer->ncallbacks - 1].func = timer_func;
    timer->callbacks[timer->ncallbacks - 1].ptr = ptr;

    return true;
}

void tp_timer_cancel(tp_timer_t *timer, timer_callback_t timer_func, void *ptr)
{
    for (int i = 0; i < timer->ncallbacks; ++i) {
        if (timer->callbacks[i].func == timer_func &&
                timer->callbacks[i].ptr == ptr) {
            memmove(&timer->callbacks[i], &timer->callbacks[i + 1],
                    sizeof(callback_t[timer->ncallbacks - i - 1]));
            --timer->ncallbacks;
            return;
        }
    }
    LOG(WTF, "callback not found");
}

int timeval_abs_delta(const struct timeval *tv1, const struct timeval *tv2)
{
    int d = tv2->tv_usec - tv1->tv_usec;
    d += (tv2->tv_sec - tv1->tv_sec) * 1000000;

    return abs(d);
}
