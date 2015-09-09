#pragma once

#include <tetrapol/frame.h>

typedef struct data_frame_priv_t data_frame_t;

data_frame_t *data_frame_create(void);

/**
  Reset internal state of data frame decoder.

  Should be called when stream is interrupted or corrupted.
  */
void data_frame_reset(data_frame_t *data_fr);

/**
  @return number of blocks in current data frame.
  */
int data_frame_blocks(data_frame_t *data_fr);

/**
  Add new decoded frame into data frame processing chain.

  @return
    2 when new data frame is decoded but some data have been discarded
        (previously unrecognizable fail reported as 0)
    1 when new data frame is decoded
    0 when more data blocks is required
    -1 when block reconstruction fails.
  */
int data_frame_push_frame(data_frame_t *data_fr, const frame_t *fr);

/**
  Get data from data_frame, data are packe into bytes.
  Unused bits in last byte are set to zero.

  @return number of bits writen into output array
  */
int data_frame_get_bytes(data_frame_t *data_fr, uint8_t *data);

void data_frame_destroy(data_frame_t *data_fr);

