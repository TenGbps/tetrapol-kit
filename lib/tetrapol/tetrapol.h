#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    TETRAPOL_BAND_VHF = 1,
    TETRAPOL_BAND_UHF = 2,
};

/** Radio channel type. */
enum {
    // TETRAPOL_RADIO_AUTO = 0,     // not supported yet
    TETRAPOL_RADIO_CCH = 1,
    TETRAPOL_RADIO_TCH = 2,
};

#ifdef __cplusplus
}
#endif
