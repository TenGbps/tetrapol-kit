#pragma once

#include <stdbool.h>
#include <tetrapol/frame.h>

typedef struct pch_priv_t pch_t;

pch_t *pch_create(void);
void pch_destroy(pch_t *pch);

/** Should be called when some frames are missing. */
void pch_reset(pch_t *pch);
bool pch_push_frame(pch_t *pch, const frame_t* fr, int frame_no);
void pch_print(pch_t *pch);
