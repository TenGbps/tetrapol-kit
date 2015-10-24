#pragma once

// Internal library functions of tetrapol.c

#include <tetrapol/addr.h>
#include <tetrapol/tetrapol.h>

enum {
    FRAME_NO_UNKNOWN = -1,
};

enum {
    LOG_CH_BCH,
    LOG_CH_DACH,
    LOG_CH_DCH,
    LOG_CH_PCH,
    LOG_CH_RACH,
    LOG_CH_RCH,
    LOG_CH_SCH,
    LOG_CH_SDCH,
    LOG_CH_VCH,
};

enum {
    TSAP_ID_UNKNOWN = -1,
};

enum {
    TSAP_REF_UNKNOWN = -1,
};

typedef struct {
    tetrapol_cfg_t cfg;
    uint64_t rx_offs;
    int frame_no;
} tpol_t;

enum {
    TPDU_TYPE_TPDU,
    TPDU_TYPE_TPDU_UI,
};

typedef struct {
    int log_ch;
    addr_t addr;
    uint8_t tpdu_type;
    int prio;
    int tsap_id;
    int tsap_ref_swmi;
    int tsap_ref_rt;
    int data_len;
    const uint8_t *data;
} tpol_tsdu_t;

tpol_t *tetrapol_get_tpol(tetrapol_t *tetrapol);
void tetrapol_evt_tsdu(tpol_t *tpol, const tpol_tsdu_t *tpol_tsdu);
