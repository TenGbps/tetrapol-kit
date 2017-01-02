#pragma once

#include <tetrapol/hdlc_frame.h>
#include <tetrapol/tp_timer.h>
#include <tetrapol/tetrapol_int.h>

typedef struct link_priv_t link_t;

link_t *link_create(tpol_t *tpol, int log_ch);
void link_destroy(link_t *link);
int link_push_hdlc_frame(link_t *link, const hdlc_frame_t *hdlc_fr);
void link_rx_glitch(link_t *link);
void link_tick(time_evt_t* te, link_t *link);

