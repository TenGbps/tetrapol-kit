#pragma once
#include <tetrapol/data_block.h>
#include <tetrapol/timer.h>

typedef struct cch_priv_t cch_t;

cch_t *cch_create(void);
void cch_destroy(cch_t *cch);

/**
  Proces decoded frame for CCH.

  @param cch
  @param data_blk
  @param frame_no current frame number or FRAME_NO_UNKNOWN.
    Set to proper value when frame with known number is detected.
  */
int cch_push_data_block(cch_t *cch, data_block_t *data_blk, int *frame_no);

/**
  Anounce framing error to CCH, allows detection of potentional framing
  synchronization loss.
  */
void cch_fr_error(cch_t *cch);

void cch_tick(const timeval_t *tv, void *cch);
