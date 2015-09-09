#define LOG_PREFIX "sdch"
#include <tetrapol/log.h>
#include <tetrapol/sdch.h>
#include <tetrapol/data_frame.h>
#include <tetrapol/hdlc_frame.h>
#include <tetrapol/misc.h>
#include <tetrapol/tsdu.h>
#include <tetrapol/terminal.h>
#include <tetrapol/system_config.h>

#include <stdlib.h>
#include <string.h>

struct sdch_priv_t {
    data_frame_t *data_fr;
    terminal_list_t *tlist;
    tsdu_t *tsdu;
};

sdch_t *sdch_create(void)
{
    sdch_t *sdch = malloc(sizeof(sdch_t));
    if (!sdch) {
        return NULL;
    }

    sdch->data_fr = data_frame_create();
    if (!sdch->data_fr) {
        goto err_data_fr;
    }

    sdch->tlist = terminal_list_create();
    if (!sdch->tlist) {
        goto err_tlist;
    }

    sdch->tsdu = NULL;

    return sdch;

err_tlist:
    data_frame_destroy(sdch->data_fr);

err_data_fr:
    free(sdch);

    return NULL;
}

void sdch_destroy(sdch_t *sdch)
{
    if (sdch) {
        data_frame_destroy(sdch->data_fr);
        terminal_list_destroy(sdch->tlist);
        tsdu_destroy(sdch->tsdu);
    }
    free(sdch);
}

bool sdch_dl_push_data_frame(sdch_t *sdch, const frame_t *fr)
{
    if (data_frame_push_frame(sdch->data_fr, fr) <= 0) {
        return false;
    }

    uint8_t data[SYS_PAR_N200_BYTES_MAX];
    const int size = data_frame_get_bytes(sdch->data_fr, data);

    hdlc_frame_t hdlc_fr;

    if (!hdlc_frame_parse(&hdlc_fr, data, size)) {
        // PAS 0001-3-3 7.4.1.9 stuffing frames are dropped, FCS does not match
        int idx = hdlc_frame_stuffing_idx(&hdlc_fr);
        if (idx == -1) {
            LOG(INFO, "HDLC: broken frame");
        } else {
            LOG(INFO, "HDLC: stuffing idx=%d", idx);
        }
        return false;
    }

    tsdu_destroy(sdch->tsdu);
    if (terminal_list_push_hdlc_frame(sdch->tlist, &hdlc_fr, &sdch->tsdu) == -1) {
        return false;
    }

    return sdch->tsdu != NULL;
}

tsdu_t *sdch_get_tsdu(sdch_t *sdch)
{
    tsdu_t *tsdu = sdch->tsdu;
    sdch->tsdu = NULL;

    return tsdu;
}

void sdch_tick(const timeval_t *tv, void *sdch)
{
    sdch_t *_sdch = sdch;
    terminal_list_tick(_sdch->tlist, tv);
}
