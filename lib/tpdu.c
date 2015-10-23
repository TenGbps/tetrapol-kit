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
    struct timeval tv;
    uint8_t id_tsap;
    uint8_t prio;
    uint8_t nsegments;  ///< total amount of segments (HDLC frames) in DU
    hdlc_frame_t *hdlc_frs[SYS_PAR_N452];
} segmented_du_t;

typedef enum {
    CONNECTION_STATE_NC = 0,    ///< not connected
    CONNECTION_STATE_CR = 1,    ///< connection request
    CONNECTION_STATE_CONNECTED = 2,
    CONNECTION_STATE_BROKEN = 3,
} connection_state_t;

typedef struct {
    connection_state_t state;
    int8_t tsap_id;
    int8_t tsap_ref_swmi;
    int8_t tsap_ref_rt;
    int seg_len;
    uint8_t segbuf[2000];
} connection_t;

struct tpdu_priv_t {
    // connections are listed by TSAP reference (SwMI side)
    // 8 normal, 7 fast connections
    connection_t conns[8+7];
    int log_ch;
    tpol_t *tpol;
};

struct tpdu_priv_ui_t {
    frame_type_t fr_type;
    segmented_du_t *seg_du[128];
    int log_ch;
    tpol_t *tpol;
};

static void connection_reset(connection_t *conn)
{
    conn->state = CONNECTION_STATE_NC;
    conn->seg_len = 0;
}

static void connection_fcr(connection_t *conn, int tsap_id, int tsap_ref)
{
    LOG(INFO, "FCR TSAP_ref: %d TSAP_id: %d", tsap_ref, tsap_id);

    if (conn->state != CONNECTION_STATE_NC) {
        LOG(INFO, "closing existing connection");
        connection_reset(conn);
    }

    conn->state = CONNECTION_STATE_CONNECTED;
    conn->tsap_id = tsap_id;
    conn->tsap_ref_swmi = tsap_ref;
    conn->tsap_ref_rt = tsap_ref;
}

static void connection_cr(connection_t *conn, int tsap_id, int tsap_ref)
{
    LOG(INFO, "CR TSAP_ref: %d TSAP_id: %d", tsap_ref, tsap_id);

    if (conn->state != CONNECTION_STATE_NC) {
        LOG(INFO, "closing existing connection");
        connection_reset(conn);
    }

    conn->state = CONNECTION_STATE_CR;
    conn->tsap_id = tsap_id;
    conn->tsap_ref_swmi = tsap_ref;
    conn->tsap_ref_rt = TSAP_REF_UNKNOWN;
}

static void connection_cc(connection_t *conn, int tsap_ref_swmi, int tsap_ref_rt)
{
    LOG(INFO, "CC TSAP_ref_SwMI=%d TSAP_ref_RT=%d", tsap_ref_swmi, tsap_ref_rt);

    if (conn->state != CONNECTION_STATE_NC) {
        LOG(INFO, "closing existing connection");
        connection_reset(conn);
    }

    conn->state = CONNECTION_STATE_CONNECTED;
    // when we receive CC we are missing CR from uplink
    conn->tsap_id = TSAP_ID_UNKNOWN;
    conn->tsap_ref_swmi = tsap_ref_swmi;
    conn->tsap_ref_rt = tsap_ref_rt;
}

// used for both DT and DTE
static int connection_dt(connection_t *conn, int tsap_ref_swmi, int tsap_ref_rt, int seg)
{
    LOG(INFO, "DT TSAP_ref_SwMI=%d TSAP_ref_RT=%d", tsap_ref_swmi, tsap_ref_rt);

    if (conn->state == CONNECTION_STATE_NC) {
        if (!seg) {
            LOG(INFO, "Repairing broken connection");
            conn->state = CONNECTION_STATE_CONNECTED;
            conn->tsap_ref_rt = tsap_ref_rt;
            conn->tsap_ref_swmi = tsap_ref_swmi;
            conn->tsap_id = TSAP_ID_UNKNOWN;
        } else {
            LOG(INFO, "Link broken, connection does not exists");
            conn->state = CONNECTION_STATE_BROKEN;
        }
        return -1;
    }
    if (conn->state == CONNECTION_STATE_BROKEN) {
        if (!seg) {
            LOG(INFO, "Repairing broken connection");
            conn->state = CONNECTION_STATE_CONNECTED;
            conn->tsap_ref_rt = tsap_ref_rt;
            conn->tsap_ref_swmi = tsap_ref_swmi;
            conn->tsap_id = TSAP_ID_UNKNOWN;
        }
        return -1;
    }
    if (conn->state == CONNECTION_STATE_CR) {
        conn->state = CONNECTION_STATE_CONNECTED;
        conn->tsap_ref_rt = tsap_ref_rt;
    }
    if (conn->tsap_ref_rt != tsap_ref_rt) {
        LOG(INFO, "Link broken, invalid RT TSAP reference");
        conn->state = CONNECTION_STATE_BROKEN;
        return -1;
    }

    return 0;
}

// should be called fot FDR, DR, DC (in pre-processing phase)
static int connection_dc_dr_fdr(connection_t *conn, int tsap_ref_swmi, int tsap_ref_rt)
{
    LOG(INFO, "FDR/DR/DC TSAP_ref_SwMI=%d TSAP_ref_RT=%d", tsap_ref_swmi, tsap_ref_rt);

    if (conn->state == CONNECTION_STATE_NC) {
        LOG(INFO, "Disconnect for not-opened connection");
        return -1;
    }
    if (conn->state == CONNECTION_STATE_BROKEN) {
        return -1;
    }
    if (conn->tsap_ref_rt != tsap_ref_rt && conn->state != CONNECTION_STATE_CR) {
        LOG(INFO, "Link broken, invalid RT TSAP reference");
        conn->state = CONNECTION_STATE_BROKEN;
        return -2;
    }

    return 0;
}

static void connection_broken(connection_t *conn)
{
    conn->state = CONNECTION_STATE_BROKEN;
}

tpdu_t *tpdu_create(tpol_t *tpol, int log_ch)
{
    tpdu_t *tpdu = calloc(1, sizeof(tpdu_t));
    if (!tpdu) {
        return NULL;
    }

    for (int i = 0; i < ARRAY_LEN(tpdu->conns); ++i) {
        connection_reset(&tpdu->conns[i]);
    }
    tpdu->tpol = tpol;
    tpdu->log_ch = log_ch;

    return tpdu;
}

int tpdu_push_hdlc_frame(tpdu_t *tpdu, const hdlc_frame_t *hdlc_fr)
{
    tpol_tsdu_t tpol_tsdu;
    tpol_tsdu.log_ch = tpdu->log_ch;
    tpol_tsdu.tpdu_type = TPDU_TYPE_TPDU;
    tpol_tsdu.prio = 0;

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

    if (par_field == 0xf) {
        LOG(WTF, "Disconnect indicated conn=%d reason=0x%02x",
                dest_ref, hdlc_fr->data[2]);
        // TODO: send disconnect indication message
        return 0;
    }
    // For downlink par_field always contains TSAP reference of sender (SwMI)
    connection_t *conn = &tpdu->conns[par_field];
    tpol_tsdu.tsap_id = conn->tsap_id;
    tpol_tsdu.tsap_ref_swmi = par_field;
    tpol_tsdu.tsap_ref_rt = conn->tsap_ref_rt;

    const uint8_t *payload = hdlc_fr->data + 2;
    int payload_len = hdlc_fr->nbits / 8 - 2;
    if (!seg) {
        if (d) {
            if (payload[0] > payload_len - 1) {
                LOG(WTF, "Invalid payload length %d > %d",
                        payload[0], payload_len - 1);
                connection_broken(conn);
                return -1;
            }
            payload_len = payload[0];
            ++payload;
        } else {
            payload_len = 0;
        }
    }

    const uint8_t code_prefix = code & TPDU_CODE_PREFIX_MASK;

    if (code_prefix != TPDU_CODE_PREFIX_MASK) {
        // TODO: use QoS
        // const uint8_t qos = (~TPDU_CODE_PREFIX_MASK) & code;

        switch(code_prefix) {
            case TPDU_CODE_CR:
                connection_cr(conn, dest_ref, par_field);
                // TODO: decode bs_ref, rt_ref, call_priority
                payload += 2;
                payload_len -= 2;
                if (payload_len < 0) {
                    LOG(WTF, "CR payload too short %d", payload_len);
                    connection_broken(conn);
                    return -1;
                }
                break;

            case TPDU_CODE_CC:
                connection_cc(conn, par_field, dest_ref);
                if (payload_len != 1) {
                    LOG(WTF, "Invalid CC lenght=%d", payload_len)
                    return -1;
                }
                LOG(INFO, "TODO: D_ACK TSDU");
                // TODO: decode bs_ref, rt_ref
                return 0;

            case TPDU_CODE_FCR:
                connection_fcr(conn, dest_ref, par_field);
                break;

            default:
                LOG(WTF, "unknown code %d", code);
        }
    } else {
        int ret_val;

        switch (code) {
            case TPDU_CODE_DR:
            case TPDU_CODE_FDR:
            case TPDU_CODE_DC:
                ret_val = connection_dc_dr_fdr(conn, par_field, dest_ref);
                if (ret_val == -1) {
                    connection_reset(conn);
                    return 0;
                }
                if (ret_val == -2) {
                    return 0;
                }
                break;

            case TPDU_CODE_DT:
            case TPDU_CODE_DTE:
                if (connection_dt(conn, par_field, dest_ref, seg) == -1) {
                    return 0;
                }
                break;

            default:
                LOG(WTF, "unknown code %d", code);
        }
    }

    if (seg) {
        if (conn->seg_len + payload_len > SIZEOF(connection_t, segbuf)) {
            LOG(WTF, "Too large TPDU, increase buffer size");
            return -1;
        }
        memcpy(&conn->segbuf[conn->seg_len], payload, payload_len);
        conn->seg_len += payload_len;
        LOG(INFO, "Segmentation part len=%d seg_len=%d dest_ref=%d",
                payload_len, conn->seg_len, dest_ref);
    } else {
        if (conn->seg_len) {
            if (conn->seg_len + payload_len > SIZEOF(connection_t, segbuf)) {
                conn->seg_len = 0;
                LOG(WTF, "Too large TPDU, increase buffer size");
                return -1;
            }
            memcpy(&conn->segbuf[conn->seg_len], payload, payload_len);
            conn->seg_len += payload_len;
            LOG(INFO, "Segmentation complete len=%d seg_len=%d dest_ref=%d",
                    payload_len, conn->seg_len, dest_ref);
            // TODO: prio, qos

            memcpy(&tpol_tsdu.addr, &hdlc_fr->addr, sizeof(tpol_tsdu.addr));
            tpol_tsdu.data_len = conn->seg_len;
            tpol_tsdu.data = conn->segbuf;
            tetrapol_evt_tsdu(tpdu->tpol, &tpol_tsdu);

            conn->seg_len = 0;
        } else {
            if (d) {
                // TODO: prio, qos

                memcpy(&tpol_tsdu.addr, &hdlc_fr->addr, sizeof(tpol_tsdu.addr));
                tpol_tsdu.data_len = payload_len;
                tpol_tsdu.data = payload;
                tetrapol_evt_tsdu(tpdu->tpol, &tpol_tsdu);
            }
        }
    }

    if (code_prefix == TPDU_CODE_PREFIX_MASK) {
        switch (code) {
            case TPDU_CODE_DR:
            case TPDU_CODE_FDR:
            case TPDU_CODE_DC:
                connection_reset(conn);
                break;

            case TPDU_CODE_DT:
            case TPDU_CODE_DTE:
                break;

            default:
                LOG(WTF, "unknown code %d", code);
        }
    }

    return 0;
}

void tpdu_destroy(tpdu_t *tpdu)
{
    free(tpdu);
}

static void tpdu_ui_segments_destroy(segmented_du_t *du)
{
    for (int i = 0; i < SYS_PAR_N452; ++i) {
        free(du->hdlc_frs[i]);
    }
    free(du);
}

tpdu_ui_t *tpdu_ui_create(tpol_t *tpol, frame_type_t fr_type, int log_ch)
{
    if (fr_type != FRAME_TYPE_DATA && fr_type != FRAME_TYPE_HR_DATA) {
        LOG(ERR, "usnupported frame type %d", fr_type);
        return NULL;
    }

    tpdu_ui_t *tpdu = calloc(1, sizeof(tpdu_ui_t));
    if (!tpdu) {
        return NULL;
    }
    tpdu->tpol = tpol;
    tpdu->fr_type = fr_type;
    tpdu->log_ch = log_ch;

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
    if (tsdu) {
        *tsdu = NULL;
    }

    if (hdlc_fr->nbits < 8) {
        LOG(WTF, "too short HDLC (%d)", hdlc_fr->nbits);
        return -1;
    }

    bool ext                    = get_bits(1, hdlc_fr->data, 0);
    const bool seg              = get_bits(1, hdlc_fr->data, 1);
    const uint8_t prio          = get_bits(2, hdlc_fr->data, 2);
    const uint8_t id_tsap       = get_bits(4, hdlc_fr->data, 4);

    tpol_tsdu_t tpol_tsdu;
    tpol_tsdu.tpdu_type = TPDU_TYPE_TPDU_UI;
    tpol_tsdu.log_ch = tpdu->log_ch;
    tpol_tsdu.prio = prio;
    tpol_tsdu.tsap_id = id_tsap;
    tpol_tsdu.tsap_ref_swmi = TSAP_REF_UNKNOWN;
    tpol_tsdu.tsap_ref_rt = TSAP_REF_UNKNOWN;

    LOG(DBG, "DU EXT=%d SEG=%d PRIO=%d ID_TSAP=%d", ext, seg, prio, id_tsap);
    if (ext == 0 && seg == 0) {
        // PAS 0001-3-3 9.5.1.2
        if ((tpdu->fr_type == FRAME_TYPE_DATA && hdlc_fr->nbits > (3*8)) ||
                (tpdu->fr_type == FRAME_TYPE_HR_DATA &&
                 hdlc_fr->nbits > (6*8))) {
            const int len = get_bits(8, hdlc_fr->data + 1, 0);

            memcpy(&tpol_tsdu.addr, &hdlc_fr->addr, sizeof(tpol_tsdu.addr));
            tpol_tsdu.data_len = len;
            tpol_tsdu.data = hdlc_fr->data + 2;
            tetrapol_evt_tsdu(tpdu->tpol, &tpol_tsdu);

            if (tsdu) {
                return tsdu_decode(hdlc_fr->data + 2, len, tsdu);
            }
            return 0;
        }
        const int len = hdlc_fr->nbits / 8 - 1;

        memcpy(&tpol_tsdu.addr, &hdlc_fr->addr, sizeof(tpol_tsdu.addr));
        tpol_tsdu.data_len = len;
        tpol_tsdu.data = hdlc_fr->data + 1;
        tetrapol_evt_tsdu(tpdu->tpol, &tpol_tsdu);

        if (tsdu) {
            return tsdu_decode(hdlc_fr->data + 1, len, tsdu);
        }
        return 0;
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
        return -1;
    }

    ext                         = get_bits(1, hdlc_fr->data + 2, 0);
    const bool res              = get_bits(1, hdlc_fr->data + 2, 1);
    const uint8_t packet_num    = get_bits(6, hdlc_fr->data + 2, 2);
    if (ext) {
        LOG(WTF, "unsupported long ext");
        return -1;
    }

    if (res) {
        LOG(WTF, "res != 0");
    }
    LOG(DBG, "UI SEGM_REF=%d, PACKET_NUM=%d", seg_ref, packet_num);

    segmented_du_t *seg_du = tpdu->seg_du[seg_ref];
    if (seg_du == NULL) {
        seg_du = calloc(1, sizeof(segmented_du_t));
        if (!seg_du) {
            return -1;
        }
        tpdu->seg_du[seg_ref] = seg_du;
        seg_du->id_tsap = id_tsap;
        seg_du->prio = prio;
    }

    if (seg_du->hdlc_frs[packet_num]) {
        // segment already recieved
        return 0;
    }

    seg_du->hdlc_frs[packet_num] = malloc(sizeof(hdlc_frame_t));
    if (!seg_du->hdlc_frs[packet_num]) {
        LOG(ERR, "ERR OOM");
        return -1;
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
        return 0;
    }

    // check if we have all segments
    for (int i = 0; i < seg_du->nsegments; ++i) {
        if (!seg_du->hdlc_frs[i]) {
            return 0;
        }
    }

    // max_segments * (sizeof(hdlc_frame_t->data) - TPDU_DU_header)
    uint8_t data[SYS_PAR_N452 * (sizeof(((hdlc_frame_t*)NULL)->data) - 3)];
    int data_len = 0;
    // collect data from all segments
    if (tpdu->fr_type == FRAME_TYPE_DATA) {
        uint8_t *d = data;
        for (int i = 0; i < seg_du->nsegments; ++i) {
            hdlc_frame_t *hfr = seg_du->hdlc_frs[i];
            int n_ext = 1;
            // skip ext headers
            while (get_bits(1, hfr->data + n_ext - 1, 0)) {
                ++n_ext;
            }
            int n;
            if (i == (seg_du->nsegments - 1)) {
                n = hfr->data[n_ext++];
                if (n > hfr->nbits / 8) {
                    LOG(WTF, "hdlc_fr.len=%d < tsdu_payload_len=%d",
                            hfr->nbits / 8, n);
                    return -1;
                }
            } else {
                n = (hfr->nbits / 8) - n_ext;
            }
            data_len += n;
            memcpy(d, &hfr->data[n_ext], n);
            d += n;
        }
    } else {    // FRAME_TYPE_HR_DATA
        LOG(WTF, "FRAME_TYPE_HR_DATA not implemented");
        // TODO
    }

    tpdu_ui_segments_destroy(seg_du);
    tpdu->seg_du[seg_ref] = NULL;

    memcpy(&tpol_tsdu.addr, &hdlc_fr->addr, sizeof(tpol_tsdu.addr));
    tpol_tsdu.data_len = data_len;
    tpol_tsdu.data = data;
    tetrapol_evt_tsdu(tpdu->tpol, &tpol_tsdu);

    if (tsdu) {
        return tsdu_decode(data, data_len, tsdu);
    }
    return 0;
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

void tpdu_rx_glitch(tpdu_t *tpdu)
{
    for (int i = 0; i < ARRAY_LEN(tpdu->conns); ++i) {
        if (tpdu->conns[i].state != CONNECTION_STATE_NC) {
            tpdu->conns[i].state = CONNECTION_STATE_BROKEN;
        }
    }
}

void tpdu_du_tick(time_evt_t *te, void *tpdu_du)
{
    tpdu_ui_t *tpdu = tpdu_du;

    // set/check T454 timer
    for (int i = 0; i < ARRAY_LEN(tpdu->seg_du); ++i) {
        if (!tpdu->seg_du[i]) {
            continue;
        }

        if (!tpdu->seg_du[i]->tv.tv_sec && !tpdu->seg_du[i]->tv.tv_usec) {
            tpdu->seg_du[i]->tv.tv_sec = te->tv.tv_sec;
            tpdu->seg_du[i]->tv.tv_usec = te->tv.tv_usec;
            continue;
        }

        if (timeval_abs_delta(&tpdu->seg_du[i]->tv, &te->tv) < SYS_PAR_T454) {
            continue;
        }

        // TODO: report error to application layer
        tpdu_ui_segments_destroy(tpdu->seg_du[i]);
        tpdu->seg_du[i] = NULL;
    }
}
