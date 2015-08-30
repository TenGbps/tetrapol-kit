#pragma once
#include <tetrapol/data_frame.h>
#include <tetrapol/hdlc_frame.h>
#include <stdint.h>

enum {
    D_CHANNEL_FREE = 0,
    D_FORCED_LISTENING = 1,
    D_VOICE_STUFFING = 2,
    D_START_SPEECH = 3,
    D_VOICE_STUFFING_2 = 4,
    U_START_SPEECH = 16,
    U_END_SPEECH_1 = 17,
    U_END_SPEECH_2 = 18,
    U_END_SPEECH_3 = 19,
    // 240-254 reserved for RT - RT signaling in direct mode
};

enum {
    LSDU_VCH_TRANSMIT_PRIORITY_ZERO = 0,
    LSDU_VCH_TRANSMIT_PRIORITY_STANDARD = 1,
    // TRANSMIT_PRIORITY_level2..15
};

typedef struct {
    uint8_t codop;
    uint8_t num_block;
    uint8_t transmit_priority : 4;
} lsdu_vch_t;

void lsdu_vch_destroy(lsdu_vch_t *lsdu);
int lsdu_vch_decode_hdlc_frame(const hdlc_frame_t *hdlc_fr, lsdu_vch_t **lsdu);
void lsdu_vch_print(const lsdu_vch_t *lsdu);

