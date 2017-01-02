#pragma once

#include <tetrapol/frame.h>
#include <tetrapol/tetrapol_int.h>
#include <tetrapol/tp_timer.h>

typedef struct tch_priv_t tch_t;

tch_t *tch_create(tpol_t *tpol);
void tch_destroy(tch_t *tch);
int tch_push_frame(tch_t *tch, const frame_t *fr);
void tch_tick(time_evt_t *te, void *tch);
