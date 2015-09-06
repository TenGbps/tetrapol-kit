#define LOG_PREFIX "frame"
#include <tetrapol/log.h>
#include <tetrapol/frame.h>
#include <stdlib.h>

struct frame_decoder_priv_t {
    int band;
    int scr;
    int fr_type;
};

frame_decoder_t *frame_decoder_create(int band, int scr, int fr_type)
{
    return NULL;
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
}
