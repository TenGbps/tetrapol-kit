#pragma once
#include <tetrapol/data_block.h>
#include <tetrapol/timer.h>

typedef struct cch_priv_t cch_t;

cch_t *cch_create(void);
void cch_destroy(cch_t *cch);
int cch_push_data_block(cch_t *cch, data_block_t *data_blk);

/**
  Anounce framing error to CCH, allows detection of potentional framing
  synchronization loss.
  */
void cch_fr_error(cch_t *cch);

void cch_tick(const timeval_t *tv, void *cch);
