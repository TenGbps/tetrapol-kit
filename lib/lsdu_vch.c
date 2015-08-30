#define LOG_PREFIX "lsdu_vch"
#include <tetrapol/log.h>
#include <tetrapol/lsdu_vch.h>
#include <stdlib.h>
#include <string.h>

static const char *codop_str[256] = {
    "D_CHANNEL_FREE",           // 0x00,
    "D_FORCED_LISTENING",       // 0x01,
    "D_VOICE_STUFFING",         // 0x02,
    "D_START_SPEECH",           // 0x03,
    "D_VOICE_STUFFING_2",       // 0x04,
    "N/A",                         // 0x05,
    "N/A",                         // 0x06,
    "N/A",                         // 0x07,
    "N/A",                         // 0x08,
    "N/A",                         // 0x09,
    "N/A",                         // 0x0a,
    "N/A",                         // 0x0b,
    "N/A",                         // 0x0c,
    "N/A",                         // 0x0d,
    "N/A",                         // 0x0e,
    "N/A",                         // 0x0f,
    "U_START_SPEECH",           // 0x10,
    "U_END_SPEECH_1",           // 0x11,
    "U_END_SPEECH_2",           // 0x12,
    "U_END_SPEECH_3",           // 0x13,
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
};

void lsdu_vch_destroy(lsdu_vch_t *lsdu)
{
    free(lsdu);
}

int lsdu_vch_decode_hdlc_frame(const hdlc_frame_t *hdlc_fr, lsdu_vch_t **lsdu)
{
    if (!lsdu) {
        LOG(WTF, "lsdu == NULL");
        return -1;
    }

    *lsdu = malloc(sizeof(lsdu_vch_t));
    if (!*lsdu) {
        return -1;
    }

    memcpy(*lsdu, hdlc_fr->data, 3);

    return 0;
}

void lsdu_vch_print(const lsdu_vch_t *lsdu)
{
    LOG_("LSDU_VCH: codop=%d (%s)", lsdu->codop, codop_str[lsdu->codop]);
    if (lsdu->codop == D_VOICE_STUFFING || lsdu->codop == D_START_SPEECH) {
        LOGF(" num_block=%d transmit_priority=%d",
                lsdu->num_block, lsdu->transmit_priority);
    }
    if (lsdu->codop == U_START_SPEECH) {
        LOGF(" transmit_priority=%d", lsdu->transmit_priority);
    }
    LOGF("\n");
}
