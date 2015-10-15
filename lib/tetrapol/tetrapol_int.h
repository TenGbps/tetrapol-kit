#pragma once

// Internal library functions of tetrapol.c

#include <tetrapol/tetrapol.h>

enum {
    FRAME_NO_UNKNOWN = -1,
};

typedef struct {
    tetrapol_cfg_t cfg;
    uint64_t rx_offs;
    int frame_no;
} tpol_t;

tpol_t *tetrapol_get_tpol(tetrapol_t *tetrapol);

