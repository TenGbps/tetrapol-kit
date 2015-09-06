#define LOG_PREFIX "cch"
#include <tetrapol/cch.h>
#include <tetrapol/log.h>
#include <tetrapol/misc.h>
#include <tetrapol/bch.h>
#include <tetrapol/pch.h>
#include <tetrapol/rch.h>
#include <tetrapol/sdch.h>
#include <stdlib.h>

struct cch_priv_t {
    int cch_mux_type;   ///< CCH multiplexing, see PAS 0001-3-3 5.1.3
    bch_t *bch;
    pch_t *pch;
    rch_t *rch;
    sdch_t *sdch;
};

cch_t *cch_create(void)
{
    cch_t *cch = malloc(sizeof(cch_t));
    if (!cch) {
        return NULL;
    }

    cch->bch = bch_create();
    if (!cch->bch) {
        goto err_bch;
    }
    cch->pch = pch_create();
    if (!cch->pch) {
        goto err_pch;
    }
    cch->rch = rch_create();
    if (!cch->rch) {
        goto err_rch;
    }
    cch->sdch = sdch_create();
    if (!cch->sdch) {
        goto err_sdch;
    }

    return cch;

err_sdch:
    rch_destroy(cch->rch);

err_rch:
    pch_destroy(cch->pch);

err_pch:
    bch_destroy(cch->bch);

err_bch:
    free(cch);
    return NULL;
}

void cch_destroy(cch_t *cch)
{
    if (cch) {
        bch_destroy(cch->bch);
        pch_destroy(cch->pch);
        rch_destroy(cch->rch);
        sdch_destroy(cch->sdch);
    }

    free(cch);
}

int cch_push_data_block(cch_t *cch, data_block_t *data_blk)
{
    LOG_IF(DBG) {
        if (!data_blk->nerrs) {
            int asbx = data_blk->data[67];
            int asby = data_blk->data[68];
            int fn0 = data_blk->data[1];
            int fn1 = data_blk->data[2];
            LOG_("OK frame_no=%03i fn=%i%i asb=%i%i data=",
                    data_blk->frame_no, fn1, fn0, asbx, asby);
        } else {
            LOG_("ERR frame_no=%03i data=", data_blk->frame_no);
        }
        char buf[64*3];
        LOGF("\t%s\n", sprint_hex(buf, data_blk->data + 3, 64));
    }

    // For decoding BCH are used always all frames, not only 0-3, 100-103
    // Firs of all for detection BCH (frame 0/100 in superblock).
    // The second reason is just to check frame synchronization.
    if (bch_push_data_block(cch->bch, data_blk)) {
        tsdu_d_system_info_t *tsdu = bch_get_tsdu(cch->bch);
        if (tsdu) {
            cch->cch_mux_type = tsdu->cell_config.mux_type;
            if (cch->cch_mux_type != CELL_CONFIG_MUX_TYPE_DEFAULT &&
                    cch->cch_mux_type != CELL_CONFIG_MUX_TYPE_TYPE_2) {
                LOG(ERR, "Unknown channel multiplexing type");
                return -1;
            }
            LOG_IF(INFO) {
                LOG_("\n");
                tsdu_print(&tsdu->base);
            }
            tsdu_destroy(&tsdu->base);
            return 0;
        }
    }

    if (data_blk->frame_no == FRAME_NO_UNKNOWN) {
        return 0;
    }

    const int fn_mod = data_blk->frame_no % 100;
    // BCH is processed above
    if (fn_mod >= 0 && fn_mod <= 3) {
        return 0;
    }

    if (fn_mod == 98 || fn_mod == 99) {
        if (pch_push_data_block(cch->pch, data_blk)) {
            LOG_IF(INFO) {
                LOG_("\n");
                pch_print(cch->pch);
            }
        }
        return 0;
    }
    if (cch->cch_mux_type == CELL_CONFIG_MUX_TYPE_TYPE_2) {
        if (fn_mod == 48 || fn_mod == 49) {
            if (pch_push_data_block(cch->pch, data_blk)) {
                LOG_IF(INFO) {
                    LOG_("\n");
                    pch_print(cch->pch);
                }
            }
            return 0;
        }
    }

    if (data_blk->frame_no % 25 == 14) {
        if (rch_push_data_block(cch->rch, data_blk)) {
            LOG_IF(INFO) {
                LOG_("\n");
                rch_print(cch->rch);
            }
        }
        return 0;
    }

    if (sdch_dl_push_data_frame(cch->sdch, data_blk)) {
        tsdu_t *tsdu = sdch_get_tsdu(cch->sdch);
        if (tsdu) {
            LOG_IF(INFO) {
                LOG_("\n");
                tsdu_print(tsdu);
            }
            tsdu_destroy(tsdu);
        }
        return 0;
    }

    return -1;
}

void cch_fr_error(cch_t *cch)
{
    pch_reset(cch->pch);
}

void cch_tick(const timeval_t *tv, void *cch)
{
}
