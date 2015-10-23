#define LOG_PREFIX "bch"
#include <tetrapol/log.h>
#include <tetrapol/bch.h>
#include <tetrapol/data_frame.h>
#include <tetrapol/hdlc_frame.h>
#include <tetrapol/misc.h>
#include <tetrapol/tpdu.h>
#include <tetrapol/system_config.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


struct bch_priv_t {
    data_frame_t *data_fr;
    tpdu_ui_t *tpdu;
    tsdu_d_system_info_t *tsdu;
    tpol_t *tpol;
};

bch_t *bch_create(tpol_t *tpol)
{
    bch_t *bch = malloc(sizeof(bch_t));
    if (!bch) {
        return NULL;
    }

    bch->data_fr = data_frame_create();
    if (!bch->data_fr) {
        free(bch);
        return NULL;
    }

    bch->tpdu = tpdu_ui_create(tpol, FRAME_TYPE_DATA, LOG_CH_BCH);
    if (!bch->tpdu) {
        free(bch);
        data_frame_destroy(bch->data_fr);
        return NULL;
    }

    bch->tsdu = NULL;
    bch->tpol = tpol;

    return bch;
}

void bch_destroy(bch_t *bch)
{
    tsdu_destroy(&bch->tsdu->base);
    data_frame_destroy(bch->data_fr);
    tpdu_ui_destroy(bch->tpdu);
    free(bch);
}

bool bch_push_frame(bch_t *bch, const frame_t *fr)
{
    if (data_frame_push_frame(bch->data_fr, fr) <= 0) {
        return false;
    }

    uint8_t tpdu_data[SYS_PAR_N200_BYTES_MAX];
    const int nblocks = data_frame_blocks(bch->data_fr);
    const int size = data_frame_get_bytes(bch->data_fr, tpdu_data);

    hdlc_frame_t hdlc_fr;
    if (!hdlc_frame_parse(&hdlc_fr, tpdu_data, size)) {
        return false;
    }

    if (hdlc_fr.command.cmd != COMMAND_UNNUMBERED_UI) {
        return false;
    }

    if (!addr_is_tti_all_st(&hdlc_fr.addr, true)) {
        if (bch->tpol->frame_no == FRAME_NO_UNKNOWN) {
            LOG_IF(DBG) {
                char buf[ADDR_PRINT_BUF_SIZE];
                LOG(DBG, "invalid address for BCH %s",
                        addr_print(buf, &hdlc_fr.addr));
            }
        }
        return false;
    }

    tsdu_t *tsdu;
    if (tpdu_ui_push_hdlc_frame2(bch->tpdu, &hdlc_fr, &tsdu) == -1) {
        return false;
    }

    if (!tsdu) {
        return false;
    }

    if (tsdu->codop != D_SYSTEM_INFO) {
        LOG(DBG, "Invalid codop for BCH 0x%02x", tsdu->codop);
        tsdu_destroy(tsdu);

        return false;
    }

    tsdu_destroy(&bch->tsdu->base);
    bch->tsdu = (tsdu_d_system_info_t *)tsdu;

    const int bch_frame_no = 100 * bch->tsdu->cell_state.bch + nblocks - 1;
    if (bch->tpol->frame_no != FRAME_NO_UNKNOWN &&
            bch->tpol->frame_no != bch_frame_no) {
        LOG(ERR, "Frame skew detected %d to %d\n",
                bch->tpol->frame_no, bch_frame_no);
    }
    bch->tpol->frame_no = bch_frame_no;

    return true;
}

tsdu_d_system_info_t *bch_get_tsdu(bch_t *bch)
{
    tsdu_d_system_info_t *tsdu = bch->tsdu;
    bch->tsdu = NULL;

    return tsdu;
}
