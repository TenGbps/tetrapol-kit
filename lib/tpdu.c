#define LOG_PREFIX "tpdu"
#include <tetrapol/log.h>
#include <tetrapol/misc.h>
#include <tetrapol/tsdu.h>
#include <tetrapol/tpdu.h>
#include <tetrapol/misc.h>

#include <stdlib.h>
#include <string.h>

enum {
    TPDU_CODE_CR = 0,
    TPDU_CODE_CC = 0x8,
    TPDU_CODE_FCR = 0x10,
    TPDU_CODE_DR = 0x18,
    TPDU_CODE_FDR = 0x19,
    TPDU_CODE_DC = 0x1a,
    TPDU_CODE_DT = 0x1b,
    TPDU_CODE_DTE = 0x1c,
};

#define TPDU_CODE_PREFIX_MASK (0x18)

typedef struct {
    timeval_t tv;
    uint8_t id_tsap;
    uint8_t prio;
    uint8_t nsegments;  ///< total amount of segments (HDLC frames) in DU
    hdlc_frame_t *hdlc_frs[SYS_PAR_N452];
} segmented_du_t;

typedef struct {
    uint8_t tsap_id;
    uint8_t tsap_ref_dl;
    uint8_t tsap_ref_ul;
} connection_t;

struct tpdu_priv_t {
    connection_t *conns[16];  // listed by TSAP reference id (SwMI side)
    connection_t *conns_fast[16];  // listed by TSAP id
};

struct tpdu_priv_ui_t {
    frame_type_t fr_type;
    segmented_du_t *seg_du[128];
};

tpdu_t *tpdu_create(void)
{
    tpdu_t *tpdu = calloc(1, sizeof(tpdu_t));
    if (!tpdu) {
        return NULL;
    }

    return tpdu;
}

static int tpdu_push_information_frame(tpdu_t *tpdu,
        const hdlc_frame_t *hdlc_fr, tsdu_t **tsdu)
{
    *tsdu = NULL;

    const bool ext              = get_bits(1, hdlc_fr->data, 0);
    const bool seg              = get_bits(1, hdlc_fr->data, 1);
    const bool d                = get_bits(1, hdlc_fr->data, 2);
    const uint8_t code          = get_bits(5, hdlc_fr->data, 3);
    const uint8_t par_field     = get_bits(4, hdlc_fr->data + 1, 0);
    const uint8_t dest_ref      = get_bits(4, hdlc_fr->data + 1, 4);

    if (ext) {
        LOG(WTF, "TPDU: ext != 0");
        return -1;
    }

    const uint8_t len = (d && !seg) ? hdlc_fr->data[2] : 0;

    LOG_IF(INFO) {
        LOG_("information cmd\n");
        LOGF("\taddr: ");
        addr_print(&hdlc_fr->addr);
        LOGF("\n\trecv_seq_no: %d send_seq_no: %d P: %d\n",
                hdlc_fr->command.information.recv_seq_no,
                hdlc_fr->command.information.send_seq_no,
                hdlc_fr->command.information.p_e);
    }

    if (seg) {
        LOG(ERR, "TPDU: TODO segmentation");
        return -1;
    }

    const uint8_t code_prefix = code & TPDU_CODE_PREFIX_MASK;

    if (code_prefix != TPDU_CODE_PREFIX_MASK) {
        const uint8_t qos = (~TPDU_CODE_PREFIX_MASK) & code;

        switch(code_prefix) {
            case TPDU_CODE_CR:
                LOG(ERR, "TODO CR seg: %d d: %d TSAP_ref: %d TSAP_id %d QoS: %d len: %d",
                        seg, d, par_field, dest_ref, qos, len);
                // check if connection exists, deallocate and WTF
                // create new connection stuct
                // set state to REQ
                break;

            case TPDU_CODE_CC:
                LOG(ERR, "TODO CC seg: %d d: %d TSAP_ref_send: %d TSAP_ref_recv: %d QoS: %d len: %d",
                        seg, d, par_field, dest_ref, qos, len);
                // check if connection exists
                break;

            case TPDU_CODE_FCR:
                LOG(ERR, "TODO FCR seg: %d d: %d TSAP_ref: %d TSAP_id: %d QoS: %d len: %d",
                        seg, d, par_field, dest_ref, qos, len);
                break;

            default:
                LOG(WTF, "unknown code %d", code);
        }
    } else {
        switch (code) {
            case TPDU_CODE_DR:
                LOG(ERR, "TODO DR d: %d TSAP_ref_send: %d TSAP_ref_recv: %d len: %d",
                        d, par_field, dest_ref, len);
                break;

            case TPDU_CODE_FDR:
                LOG(ERR, "TODO FDR d: %d TSAP_ref_send: %d TSAP_ref_recv: %d len: %d",
                        d, par_field, dest_ref, len);
                break;

            case TPDU_CODE_DC:
                LOG(ERR, "TODO DC TSAP_ref_send: %d TSAP_ref_recv: %d",
                        par_field, dest_ref);
                break;

            case TPDU_CODE_DT:
                LOG(ERR, "TODO DT seg: %d d: %d TSAP_ref_send: %d TSAP_ref_recv: %d, len: %d",
                        seg, d, par_field, dest_ref, len);
                break;

            case TPDU_CODE_DTE:
                LOG(ERR, "TODO DTE TSAP_ref_send: %d TSAP_ref_recv: %d, len: %d",
                        par_field, dest_ref, len);
                break;

            default:
                LOG(WTF, "unknown code %d", code);
        }
    }

    if (d) {
        // TODO: prio, qos
        return tsdu_d_decode(hdlc_fr->data + 3, len, 0, dest_ref, tsdu);
    }

    return -1;
}

int tpdu_push_hdlc_frame(tpdu_t *tpdu, const hdlc_frame_t *hdlc_fr, tsdu_t **tsdu)
{
    return tpdu_push_information_frame(tpdu, hdlc_fr, tsdu);
}

void tpdu_destroy(tpdu_t *tpdu)
{
    free(tpdu);
}

void tpdu_ui_segments_destroy(segmented_du_t *du)
{
    for (int i = 0; i < SYS_PAR_N452; ++i) {
        free(du->hdlc_frs[i]);
    }
    free(du);
}

tpdu_ui_t *tpdu_ui_create(frame_type_t fr_type)
{
    if (fr_type != FRAME_TYPE_DATA && fr_type != FRAME_TYPE_HR_DATA) {
        LOG(ERR, "usnupported frame type %d", fr_type);
        return NULL;
    }

    tpdu_ui_t *tpdu = calloc(1, sizeof(tpdu_ui_t));
    if (!tpdu) {
        return NULL;
    }
    tpdu->fr_type = fr_type;

    return tpdu;
}

void tpdu_ui_destroy(tpdu_ui_t *tpdu)
{
    for (int i = 0; i < ARRAY_LEN(tpdu->seg_du); ++i) {
        if (!tpdu->seg_du[i]) {
            continue;
        }
        tpdu_ui_segments_destroy(tpdu->seg_du[i]);
    }
    free(tpdu);
}

static int tpdu_ui_push_hdlc_frame_(tpdu_ui_t *tpdu,
        const hdlc_frame_t *hdlc_fr, tsdu_t **tsdu, bool allow_seg)
{
    *tsdu = NULL;

    if (hdlc_fr->nbits < 8) {
        LOG(WTF, "too short HDLC (%d)", hdlc_fr->nbits);
        return -1;
    }

    bool ext                    = get_bits(1, hdlc_fr->data, 0);
    const bool seg              = get_bits(1, hdlc_fr->data, 1);
    const uint8_t prio          = get_bits(2, hdlc_fr->data, 2);
    const uint8_t id_tsap       = get_bits(4, hdlc_fr->data, 4);

    LOG(DBG, "DU EXT=%d SEG=%d PRIO=%d ID_TSAP=%d", ext, seg, prio, id_tsap);
    if (ext == 0 && seg == 0) {
        // PAS 0001-3-3 9.5.1.2
        if ((tpdu->fr_type == FRAME_TYPE_DATA && hdlc_fr->nbits > (3*8)) ||
                (tpdu->fr_type == FRAME_TYPE_HR_DATA &&
                 hdlc_fr->nbits > (6*8))) {
            const int len = get_bits(8, hdlc_fr->data + 1, 0);
            return tsdu_d_decode(hdlc_fr->data + 2, len, prio, id_tsap, tsdu);
        }
        const int len = hdlc_fr->nbits / 8 - 1;
        return tsdu_d_decode(hdlc_fr->data + 1, len, prio, id_tsap, tsdu);
    }

    if (ext != 1) {
        LOG(WTF, "unsupported ext and seg combination");
        return -1;
    }

    if (!allow_seg) {
        LOG(DBG, "Segmentation not allowed for BCH");
        return -1;
    }

    ext                         = get_bits(1, hdlc_fr->data + 1, 0);
    uint8_t seg_ref             = get_bits(7, hdlc_fr->data + 1, 1);
    if (!ext) {
        LOG(WTF, "unsupported short ext");
        return false;
    }

    ext                         = get_bits(1, hdlc_fr->data + 2, 0);
    const bool res              = get_bits(1, hdlc_fr->data + 2, 1);
    const uint8_t packet_num    = get_bits(6, hdlc_fr->data + 2, 2);
    if (ext) {
        LOG(WTF, "unsupported long ext");
        return false;
    }

    if (res) {
        LOG(WTF, "res != 0");
    }
    LOG(DBG, "UI SEGM_REF=%d, PACKET_NUM=%d", seg_ref, packet_num);

    segmented_du_t *seg_du = tpdu->seg_du[seg_ref];
    if (seg_du == NULL) {
        seg_du = calloc(1, sizeof(segmented_du_t));
        if (!seg_du) {
            return false;
        }
        tpdu->seg_du[seg_ref] = seg_du;
        seg_du->id_tsap = id_tsap;
        seg_du->prio = prio;
    }

    if (seg_du->hdlc_frs[packet_num]) {
        // segment already recieved
        return false;
    }

    seg_du->hdlc_frs[packet_num] = malloc(sizeof(hdlc_frame_t));
    if (!seg_du->hdlc_frs[packet_num]) {
        LOG(ERR, "ERR OOM");
        return false;
    }
    memcpy(seg_du->hdlc_frs[packet_num], hdlc_fr, sizeof(hdlc_frame_t));

    if (seg == 0) {
        seg_du->nsegments = packet_num + 1;
    }

    // reset T454 timer
    seg_du->tv.tv_sec = 0;
    seg_du->tv.tv_usec = 0;

    // last segment is still missing
    if (!seg_du->nsegments) {
        return false;
    }

    // check if we have all segments
    for (int i = 0; i < seg_du->nsegments; ++i) {
        if (!seg_du->hdlc_frs[i]) {
            return false;
        }
    }

    // max_segments * (sizeof(hdlc_frame_t->data) - TPDU_DU_header)
    uint8_t data[SYS_PAR_N452 * (sizeof(((hdlc_frame_t*)NULL)->data) - 3)];
    int nbits = 0;
    // collect data from all segments
    if (tpdu->fr_type == FRAME_TYPE_DATA) {
        uint8_t *d = data;
        for (int i = 0; i < seg_du->nsegments; ++i) {
            hdlc_fr = seg_du->hdlc_frs[i];
            int n_ext = 1;
            // skip ext headers
            while (get_bits(1, hdlc_fr->data + n_ext - 1, 0)) {
                ++n_ext;
            }
            int n;
            if (i == (seg_du->nsegments - 1)) {
                n = hdlc_fr->data[n_ext++] * 8;
            } else {
                n = hdlc_fr->nbits - 8 * n_ext;
            }
            nbits += n;
            memcpy(d, &hdlc_fr->data[n_ext], n / 8);
            d += n / 8;
        }
    } else {    // FRAME_TYPE_HR_DATA
        // TODO
    }

    tpdu_ui_segments_destroy(seg_du);
    tpdu->seg_du[seg_ref] = NULL;

    return tsdu_d_decode(data, nbits / 8, prio, id_tsap, tsdu);
}

int tpdu_ui_push_hdlc_frame(tpdu_ui_t *tpdu, const hdlc_frame_t *hdlc_fr,
        tsdu_t **tsdu)
{
    return tpdu_ui_push_hdlc_frame_(tpdu, hdlc_fr, tsdu, true);
}

int tpdu_ui_push_hdlc_frame2(tpdu_ui_t *tpdu, const hdlc_frame_t *hdlc_fr,
        tsdu_t **tsdu)
{
    return tpdu_ui_push_hdlc_frame_(tpdu, hdlc_fr, tsdu, false);
}

void tpdu_du_tick(const timeval_t *tv, void *tpdu_du)
{
    tpdu_ui_t *tpdu = tpdu_du;

    // set/check T454 timer
    for (int i = 0; i < ARRAY_LEN(tpdu->seg_du); ++i) {
        if (!tpdu->seg_du[i]) {
            continue;
        }

        if (!tpdu->seg_du[i]->tv.tv_sec && !tpdu->seg_du[i]->tv.tv_usec) {
            tpdu->seg_du[i]->tv.tv_sec = tv->tv_sec;
            tpdu->seg_du[i]->tv.tv_usec = tv->tv_usec;
            continue;
        }

        if (timeval_abs_delta(&tpdu->seg_du[i]->tv, tv) < SYS_PAR_T454) {
            continue;
        }

        // TODO: report error to application layer
        tpdu_ui_segments_destroy(tpdu->seg_du[i]);
        tpdu->seg_du[i] = NULL;
    }
}
