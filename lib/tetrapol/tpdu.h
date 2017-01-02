#pragma once

#include <tetrapol/hdlc_frame.h>
#include <tetrapol/data_frame.h>
#include <tetrapol/tetrapol_int.h>
#include <tetrapol/tsdu.h>
#include <tetrapol/tp_timer.h>

#include <stdbool.h>
#include <stdint.h>

typedef struct tpdu_priv_t tpdu_t;
typedef struct tpdu_priv_ui_t tpdu_ui_t;

tpdu_t *tpdu_create(tpol_t *tpol, int log_ch);
int tpdu_push_hdlc_frame(tpdu_t *tpdu, const hdlc_frame_t *hdlc_fr);

/**
  Should be called when underlaying layers detect uncorrectable error.
  It flushes all buffers and pospone decoding until data integrity can be
  ensured.
  */
void tpdu_rx_glitch(tpdu_t *tpdu);

void tpdu_destroy(tpdu_t *tpdu);
void tpdu_du_tick(time_evt_t *te, void *tpdu_du);

tpdu_ui_t *tpdu_ui_create(tpol_t *tpol, frame_type_t fr_type, int log_ch);
void tpdu_ui_destroy(tpdu_ui_t *tpdu);

/**
 * @brief tpdu_ui_push_hdlc_frame
 * Process HDLC frame, optionaly compose frame from segments.
 * @param tpdu
 * @param hdlc_fr
 * @param tsdu Set to pointer to decoded TSDU if available, NULL otherwise.
 *
 * @return 0 on sucess, -1 on error
 */
int tpdu_ui_push_hdlc_frame(tpdu_ui_t *tpdu, const hdlc_frame_t *hdlc_fr, tsdu_t **tsdu);

/**
 * @brief tpdu_ui_push_hdlc_frame2
 * Process HDLC frame, does not allow segmented frames.
 * @param tpdu
 * @param hdlc_fr
 * @param tsdu Set to pointer to decoded TSDU if available, NULL otherwise.
 *
 * @return 0 on sucess, -1 on error
 */
int tpdu_ui_push_hdlc_frame2(tpdu_ui_t *tpdu, const hdlc_frame_t *hdlc_fr, tsdu_t **tsdu);
