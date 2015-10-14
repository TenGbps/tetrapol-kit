#define LOG_PREFIX "tetrapol"

#include <tetrapol/log.h>
#include <tetrapol/tetrapol_int.h>
#include <tetrapol/tsdu.h>

#include <stdlib.h>
#include <string.h>

struct tetrapol_priv_t {
    tpol_t tpol;
};

tetrapol_t *tetrapol_create(const tetrapol_cfg_t *cfg)
{
    if (cfg->band != TETRAPOL_BAND_VHF && cfg->band != TETRAPOL_BAND_UHF) {
        LOG(ERR, "Invalid value for parametter band=%d", cfg->band);
        return NULL;
    }

    if (cfg->radio_ch_type != TETRAPOL_RADIO_CCH &&
            cfg->radio_ch_type != TETRAPOL_RADIO_TCH) {
        LOG(ERR, "Invalid value for parameter radio_ch_type=%d",
                cfg->radio_ch_type);
        return NULL;
    }

    tetrapol_t *tetrapol = malloc(sizeof(tetrapol_t));
    if (!tetrapol) {
        return NULL;
    }

    memcpy(&tetrapol->tpol.cfg, cfg, sizeof(tetrapol_cfg_t));
    tetrapol->tpol.rx_offs = 0;
    tetrapol->tpol.frame_no = FRAME_NO_UNKNOWN;

    return tetrapol;
}

void tetrapol_destroy(tetrapol_t *tetrapol)
{
    free(tetrapol);
}

const tetrapol_cfg_t *tetrapol_get_cfg(tetrapol_t *tetrapol)
{
    return &tetrapol->tpol.cfg;
}

tpol_t *tetrapol_get_tpol(tetrapol_t *tetrapol)
{
    return (tpol_t *)tetrapol;
}

void tetrapol_evt_tsdu(tpol_t *tpol, const tpol_tsdu_t *tpol_tsdu)
{
    if (tpol_tsdu->log_ch == LOG_CH_BCH) {
        if (tpol_tsdu->data_len <= 0) {
            return;
        }
        if (tpol_tsdu->data[0] != D_SYSTEM_INFO) {
            return;
        }
    }

    tsdu_t *tsdu = NULL;
    tsdu_d_decode(tpol_tsdu->data, tpol_tsdu->data_len, tpol_tsdu->prio,
            tpol_tsdu->tsap_id, &tsdu);
    if (tsdu) {
        LOG_IF(INFO) {
            LOG_("\n");
            tsdu_print(tsdu);
        }
        tsdu_destroy(tsdu);
    }
}
