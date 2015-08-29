#pragma once
#include <tetrapol/data_block.h>
#include <tetrapol/timer.h>

typedef struct tch_priv_t tch_t;

tch_t *tch_create(void);
void tch_destroy(tch_t *tch);
int tch_push_data_block(tch_t *tch, data_block_t *data_blk);
void tch_tick(const timeval_t *tv, void *tch);
