#define LOG_PREFIX "tsdu_json"

#include <tetrapol/log.h>
#include <tetrapol/misc.h>
#include <tetrapol/tsdu_json.h>

enum {
    /// buffer large enough to satisfy all internal sprintf
    SPRINTF_BUF_LEN = 32768,
};

static const char *addr_json(char *buf, const addr_t *addr)
{
    sprintf(buf, "{ \"z\": %d, \"y\": %d, \"x\": %d }",
            addr->z, addr->y, addr->x);
    return buf;
}

void tsdu_json(const tpol_t *tpol, const tpol_tsdu_t *tsdu)
{
    printf("{ \"event\": \"tsdu\", ");
    printf("\"rx_offs\": %lu, ", tpol->rx_offs);

    printf("\"tsdu\": { ");
    {
        char buf[SPRINTF_BUF_LEN];  ///< buffer for sprintf

        if (tpol->frame_no != FRAME_NO_UNKNOWN) {
            printf("\"frame_no\": %d, ", tpol->frame_no);
        } else {
            printf("\"frame_no\": null, ");
        }

        const char *log_ch_str;
        switch (tsdu->log_ch) {
            case LOG_CH_BCH:    log_ch_str = "BCH";     break;
            case LOG_CH_DACH:   log_ch_str = "DACH";    break;
            case LOG_CH_PCH:    log_ch_str = "PCH";     break;
            case LOG_CH_RACH:   log_ch_str = "RACH";    break;
            case LOG_CH_RCH:    log_ch_str = "RCH";     break;
            case LOG_CH_SDCH:   log_ch_str = "SDCH";    break;
            case LOG_CH_SCH:    log_ch_str = "SCH";     break;
            case LOG_CH_VCH:    log_ch_str = "VCH";     break;

            default:
                log_ch_str = "FIXME";
        };
        printf("\"log_ch\": \"%s\", ", log_ch_str);
        printf("\"addr\": %s, ", addr_json(buf, &tsdu->addr));

        const char *tpdu_type;
        switch (tsdu->tpdu_type) {
            case TPDU_TYPE_TPDU:    tpdu_type = "TPDU";     break;
            case TPDU_TYPE_TPDU_UI: tpdu_type = "TPDU_UI";  break;
            default:                tpdu_type = "FIXME";
        };
        printf("\"tpdu_type\": \"%s\", ", tpdu_type);

        if (tsdu->tsap_id != TSAP_ID_UNKNOWN) {
            printf("\"tsap_id\": %d, ", tsdu->tsap_id);
        } else {
            printf("\"tsap_id\": null, ");
        }

        if (tsdu->tpdu_type == TPDU_TYPE_TPDU) {
            if (tsdu->tsap_ref_swmi != TSAP_REF_UNKNOWN) {
                printf("\"tsap_ref_swmi\": %d, ", tsdu->tsap_ref_swmi);
            } else {
                printf("\"tsap_ref_swmi\": null, ");
            }
            if (tsdu->tsap_ref_rt != TSAP_REF_UNKNOWN) {
                printf("\"tsap_ref_rt\": %d, ", tsdu->tsap_ref_rt);
            } else {
                printf("\"tsap_ref_rt\": null, ");
            }
        } else if (tsdu->tpdu_type == TPDU_TYPE_TPDU_UI) {
        }

        if ( (2 * tsdu->data_len + 1) <= sizeof(buf)) {
            printf("\"data\": { \"encoding\": \"hex\", \"value\": \"%s\" } ",
                    sprint_hex2(buf, tsdu->data, tsdu->data_len));
        } else {
            printf("\"data\": null");
        }
    }
    printf("} ");

    printf("}\n");
}
