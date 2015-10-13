#include <tetrapol/tetrapol.h>
#include <tetrapol/log.h>

#include <stdlib.h>
#include <string.h>

struct tetrapol_priv_t {
    tetrapol_cfg_t cfg;
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

    memcpy(&tetrapol->cfg, cfg, sizeof(tetrapol_cfg_t));

    return tetrapol;
}

void tetrapol_destroy(tetrapol_t *tetrapol)
{
    free(tetrapol);
}

const tetrapol_cfg_t *tetrapol_get_cfg(tetrapol_t *tetrapol)
{
    return &tetrapol->cfg;
}
