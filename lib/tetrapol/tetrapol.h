#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    TETRAPOL_BAND_VHF = 1,
    TETRAPOL_BAND_UHF = 2,
};

/** Transmission direcion uplink/downlink. */
enum {
    DIR_DOWNLINK = 1,
    DIR_UPLINK = 2,
};

/** Radio channel type. */
enum {
    // TETRAPOL_RADIO_AUTO = 0,     // not supported yet
    TETRAPOL_RADIO_CCH = 1,
    TETRAPOL_RADIO_TCH = 2,
};

typedef struct {
    uint8_t band;
    uint8_t dir;
    uint8_t radio_ch_type;
} tetrapol_cfg_t;

typedef struct tetrapol_priv_t tetrapol_t;

tetrapol_t *tetrapol_create(const tetrapol_cfg_t *cfg);
void tetrapol_destroy(tetrapol_t *tetrapol);
const tetrapol_cfg_t *tetrapol_get_cfg(tetrapol_t *tetrapol);

#ifdef __cplusplus
}
#endif
