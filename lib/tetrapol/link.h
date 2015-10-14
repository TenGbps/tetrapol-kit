#pragma once

#include <tetrapol/hdlc_frame.h>
#include <tetrapol/timer.h>
#include <tetrapol/tetrapol_int.h>
#include <tetrapol/tsdu.h>

typedef struct link_priv_t link_t;

link_t *link_create(tpol_t *tpol);
void link_destroy(link_t *link);
int link_push_hdlc_frame(link_t *link, const hdlc_frame_t *hdlc_fr, tsdu_t **tsdu);
void link_rx_glitch(link_t *link);
void link_tick(time_evt_t* te, link_t *link);

