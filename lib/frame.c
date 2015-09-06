#define LOG_PREFIX "frame"
#include <tetrapol/log.h>
#include <tetrapol/frame.h>
#include <stdlib.h>
#include <string.h>

struct frame_decoder_priv_t {
    int band;
    int scr;
    int fr_type;
};

/**
  PAS 0001-2 6.1.5.1
  PAS 0001-2 6.2.5.1
  PAS 0001-2 6.3.4.1

  Scrambling sequence was generated by this python3 script

  s = [1, 1, 1, 1, 1, 1, 1]
  for k in range(len(s), 127):
    s.append(s[k-1] ^ s[k-7])
  for i in range(len(s)):
    print(s[i], end=", ")
    if i % 8 == 7:
      print()
  */
static uint8_t scramb_table[127] = {
    1, 1, 1, 1, 1, 1, 1, 0,
    1, 0, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 1, 1, 0, 1,
    1, 1, 0, 1, 0, 0, 1, 0,
    1, 1, 0, 0, 0, 1, 1, 0,
    1, 1, 1, 1, 0, 1, 1, 0,
    1, 0, 1, 1, 0, 1, 1, 0,
    0, 1, 0, 0, 1, 0, 0, 0,
    1, 1, 1, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 0, 0,
    1, 0, 1, 0, 1, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 0,
    1, 0, 0, 1, 1, 1, 1, 0,
    0, 0, 1, 0, 1, 0, 0, 0,
    0, 1, 1, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0,
};

static void frame_descramble(uint8_t *fr_data_tmp, const uint8_t *fr_data,
        int scr)
{
    if (scr == 0) {
        memcpy(fr_data_tmp, fr_data, FRAME_DATA_LEN);
        return;
    }

    for(int k = 0 ; k < FRAME_DATA_LEN; k++) {
        fr_data_tmp[k] = fr_data[k] ^ scramb_table[(k + scr) % 127];
    }
}

frame_decoder_t *frame_decoder_create(int band, int scr, int fr_type)
{
    frame_decoder_t *fd = malloc(sizeof(frame_decoder_t));
    if (!fd) {
        return NULL;
    }

    frame_decoder_reset(fd, band, scr, fr_type);

    return fd;
}

void frame_decoder_destroy(frame_decoder_t *fd)
{
    free(fd);
}

void frame_decoder_reset(frame_decoder_t *fd, int band, int scr, int fr_type)
{
    fd->band = band;
    fd->scr = scr;
    fd->fr_type = fr_type;
}

void frame_decoder_set_scr(frame_decoder_t *fd, int scr)
{
    fd->scr = scr;
}

void frame_decoder_decode(frame_decoder_t *fd, frame_t *fr, const uint8_t *fr_data)
{
    uint8_t fr_data_tmp[FRAME_DATA_LEN];

    frame_descramble(fr_data_tmp, fr_data, fd->scr);
}
