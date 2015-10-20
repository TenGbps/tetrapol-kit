#pragma once

// Internal library functions of tetrapol.c

#include <tetrapol/tetrapol.h>

typedef struct {
    tetrapol_cfg_t cfg;
} tpol_t;

tpol_t *tetrapol_get_tpol(tetrapol_t *tetrapol);

