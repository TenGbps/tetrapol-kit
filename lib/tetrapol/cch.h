#pragma once
#include <tetrapol/frame.h>
#include <tetrapol/tetrapol_int.h>
#include <tetrapol/tp_timer.h>

typedef struct cch_priv_t cch_t;

cch_t *cch_create(tpol_t *tpol);
void cch_destroy(cch_t *cch);

/**
  Proces decoded frame for CCH.

  @param cch
  @param frame
  */
int cch_push_frame(cch_t *cch, const frame_t *fr);

/**
  Anounce framing error to CCH, allows detection of potentional framing
  synchronization loss.
  */
void cch_fr_error(cch_t *cch);

void cch_tick(time_evt_t *te, void *cch);
