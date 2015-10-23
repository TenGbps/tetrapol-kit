#define LOG_PREFIX "sdch"
#include <tetrapol/log.h>
#include <tetrapol/sdch.h>
#include <tetrapol/data_frame.h>
#include <tetrapol/hdlc_frame.h>
#include <tetrapol/misc.h>
#include <tetrapol/terminal.h>
#include <tetrapol/system_config.h>

#include <stdlib.h>
#include <string.h>

struct sdch_priv_t {
    data_frame_t *data_fr;
    terminal_list_t *tlist;
    bool rx_glitch;
    // This is used for re-sending tick event with changed state
    // do not allocate or release.
    time_evt_t *te;
};

sdch_t *sdch_create(tpol_t *tpol)
{
    sdch_t *sdch = malloc(sizeof(sdch_t));
    if (!sdch) {
        return NULL;
    }

    sdch->te = NULL;

    sdch->data_fr = data_frame_create();
    if (!sdch->data_fr) {
        goto err_data_fr;
    }

    sdch->tlist = terminal_list_create(tpol, LOG_CH_SDCH);
    if (!sdch->tlist) {
        goto err_tlist;
    }

    sdch->rx_glitch = false;

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
    }
    free(sdch);
}

bool sdch_dl_push_data_frame(sdch_t *sdch, const frame_t *fr)
{
    int res = data_frame_push_frame(sdch->data_fr, fr);

    if (res < 0) {
        sdch->rx_glitch = true;
    }

    if (res <= 0) {
        return false;
    }

    if (res == 2) {
        terminal_list_rx_glitch(sdch->tlist);
    }

    uint8_t data[SYS_PAR_N200_BYTES_MAX];
    const int size = data_frame_get_bytes(sdch->data_fr, data);

    hdlc_frame_t hdlc_fr;

    if (!hdlc_frame_parse(&hdlc_fr, data, size)) {
        // PAS 0001-3-3 7.4.1.9 stuffing frames are dropped, FCS does not match
        int idx = hdlc_frame_stuffing_idx(&hdlc_fr);
        if (idx == -1) {
            sdch->rx_glitch = true;
            LOG(INFO, "HDLC: broken frame");
        } else {
            LOG(INFO, "HDLC: stuffing idx=%d", idx);
        }
        return false;
    }

    return terminal_list_push_hdlc_frame(sdch->tlist, &hdlc_fr) != -1;
}

void sdch_tick(time_evt_t *te, void *sdch)
{
    sdch_t *sdch_ = sdch;
    sdch_->te = te;
    te->rx_glitch |= sdch_->rx_glitch;
    sdch_->rx_glitch = false;
    terminal_list_tick(sdch_->tlist, te);
}
