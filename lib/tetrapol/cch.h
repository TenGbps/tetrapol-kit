#pragma once
#include <tetrapol/frame.h>
#include <tetrapol/timer.h>

typedef struct cch_priv_t cch_t;

cch_t *cch_create(void);
void cch_destroy(cch_t *cch);

/**
  Proces decoded frame for CCH.

  @param cch
  @param frame
  @param frame_no current frame number or FRAME_NO_UNKNOWN.
    Set to proper value when frame with known number is detected.
  */
int cch_push_frame(cch_t *cch, const frame_t *fr, int *frame_no);

/**
  Anounce framing error to CCH, allows detection of potentional framing
  synchronization loss.
  */
void cch_fr_error(cch_t *cch);

void cch_tick(time_evt_t *te, void *cch);
