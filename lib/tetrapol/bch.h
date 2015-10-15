#pragma once

#include <tetrapol/frame.h>
#include <tetrapol/tetrapol_int.h>
#include <tetrapol/tsdu.h>

typedef struct bch_priv_t bch_t;

bch_t *bch_create(tpol_t *tpol);
void bch_destroy(bch_t *bch);
bool bch_push_frame(bch_t *bch, const frame_t *fr);
tsdu_d_system_info_t *bch_get_tsdu(bch_t *bch);
