#pragma once

#include <stdbool.h>
#include <tetrapol/frame.h>
#include <tetrapol/tetrapol_int.h>

typedef struct pch_priv_t pch_t;

pch_t *pch_create(tpol_t *tpol);
void pch_destroy(pch_t *pch);

/** Should be called when some frames are missing. */
void pch_reset(pch_t *pch);
bool pch_push_frame(pch_t *pch, const frame_t* fr);
void pch_print(pch_t *pch);
