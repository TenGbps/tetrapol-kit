#define LOG_PREFIX "lsdu_vch"
#include <tetrapol/log.h>
#include <tetrapol/lsdu_vch.h>
#include <stdlib.h>

void lsdu_vch_destroy(lsdu_vch_t *lsdu)
{
    free(lsdu);
}

int lsdu_vch_decode_hdlc_frame(const hdlc_frame_t *hdlc_fr, lsdu_vch_t **lsdu)
{
    return -1;
}

void lsdu_vch_print(const lsdu_vch_t *lsdu)
{
    LOG(WTF, "TODO");
}
