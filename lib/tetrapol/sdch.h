#pragma once

#include <tetrapol/frame.h>
#include <tetrapol/tetrapol_int.h>
#include <tetrapol/tp_timer.h>

#include <stdbool.h>

typedef struct sdch_priv_t sdch_t;

sdch_t *sdch_create(tpol_t *tpol);
void sdch_destroy(sdch_t *sdch);
bool sdch_dl_push_data_frame(sdch_t *sdch, const frame_t *fr);
void sdch_tick(time_evt_t *te, void *sdch);
