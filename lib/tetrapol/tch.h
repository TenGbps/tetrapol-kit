#pragma once
#include <tetrapol/frame.h>
#include <tetrapol/timer.h>

typedef struct tch_priv_t tch_t;

tch_t *tch_create(void);
void tch_destroy(tch_t *tch);
int tch_push_frame(tch_t *tch, const frame_t *fr);
void tch_tick(const timeval_t *tv, void *tch);
