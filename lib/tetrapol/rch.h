#pragma once

#include <tetrapol/frame.h>
#include <tetrapol/tetrapol_int.h>
#include <stdbool.h>

typedef struct rch_priv_t rch_t;

rch_t *rch_create(tpol_t *tpol);
void rch_destroy(rch_t *rch);
bool rch_push_frame(rch_t *rch, const frame_t *fr);
void rch_print(const rch_t *rch);
