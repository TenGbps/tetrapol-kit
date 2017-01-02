#pragma once

#include <stdbool.h>
#include <sys/time.h>

typedef struct {
    struct timeval tv;
    bool rx_glitch;
} time_evt_t;
typedef struct tp_timer_priv_t tp_timer_t;

typedef void (*timer_callback_t)(time_evt_t *te, void *ptr);

tp_timer_t *tp_timer_create(void);
void tp_timer_destroy(tp_timer_t *timer);
void tp_timer_tick(tp_timer_t *timer, bool rx_glitch, int usec);
bool tp_timer_register(tp_timer_t *timer, timer_callback_t timer_func, void *ptr);
void tp_timer_cancel(tp_timer_t *timer, timer_callback_t timer_func, void *ptr);

/**
 * @brief time_delta Compute difference in two timestamps (us)
 * @param tv1
 * @param tv2
 * @return delta t in us
 */
int timeval_abs_delta(const struct timeval *tv1, const struct timeval *tv2);
