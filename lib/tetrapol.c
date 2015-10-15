#define LOG_PREFIX "tetrapol"

#include <tetrapol/log.h>
#include <tetrapol/tetrapol_int.h>

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
