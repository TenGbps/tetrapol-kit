#define LOG_PREFIX "tch"
#include <tetrapol/tch.h>
#include <tetrapol/log.h>
#include <tetrapol/sdch.h>
#include <stdlib.h>

struct tch_priv_t {
    sdch_t *sch;
    sdch_t *vch;
    bool rx_glitch;
    tpol_t *tpol;
    // sch_ti;
};

tch_t *tch_create(tpol_t *tpol)
{
    tch_t *tch = malloc(sizeof(tch_t));
    if (!tch) {
        return NULL;
    }

    tch->rx_glitch = false;

    tch->sch = sdch_create(tpol);
    if (!tch->sch) {
        free(tch);
        return NULL;
    }

    tch->vch = sdch_create(tpol);
    if (!tch->vch) {
        sdch_destroy(tch->sch);
        free(tch);
        return NULL;
    }
    tch->tpol = tpol;

    return tch;
}

void tch_destroy(tch_t *tch)
{
    if (tch) {
        sdch_destroy(tch->sch);
        sdch_destroy(tch->vch);
    }

    free(tch);
}

int tch_push_frame(tch_t *tch, const frame_t *fr)
{
    if (fr->broken) {
        LOG(INFO, "Broken frame");
        tch->rx_glitch = true;
        return -1;
    }

    if (fr->fr_type == FRAME_TYPE_VOICE) {
        LOG(INFO,"VOICE FRAME asb=%i", (fr->voice.asb[0] << 1) | fr->voice.asb[1]);
        return 0;
    }

    if (fr->fr_type != FRAME_TYPE_DATA) {
        LOG(WTF, "not a data frame");
        tch->rx_glitch = true;
        return -1;
    }

    if (fr->data.asb[0]) {
        sdch_dl_push_data_frame(tch->vch, fr);
        return 0;
    }

    // TODO: separate SCH and SCH_TI

    if (fr->fr_type != FRAME_TYPE_DATA) {
        LOG(WTF, "data block expected");
        tch->rx_glitch = true;
        return -1;
    }

    sdch_dl_push_data_frame(tch->sch, fr);

    return 0;
}

void tch_tick(time_evt_t *te, void *tch_)
{
    tch_t *tch = tch_;
    te->rx_glitch |= tch->rx_glitch;
    tch->rx_glitch = false;
    sdch_tick(te, tch->sch);
}
