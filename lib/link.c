#define LOG_PREFIX "link"

#include <tetrapol/link.h>
#include <tetrapol/log.h>
#include <tetrapol/lsdu_cd.h>
#include <tetrapol/misc.h>
#include <tetrapol/tpdu.h>
#include <tetrapol/lsdu_vch.h>

#include <stdlib.h>

struct link_priv_t {
    tpdu_t *tpdu;
    tpdu_ui_t *tpdu_ui;
    uint8_t v_r;    ///< v(r) PAS 0001-3-3 7.5.4.22
};

link_t *link_create(void)
{
    link_t *link = malloc(sizeof(link_t));
    if (!link) {
        return NULL;
    }

    link->tpdu_ui = tpdu_ui_create(FRAME_TYPE_DATA);
    if (!link->tpdu_ui) {
        free(link);
        return NULL;
    }

    link->tpdu = tpdu_create();
    if (!link->tpdu) {
        tpdu_ui_destroy(link->tpdu_ui);
        free(link);
        return NULL;
    }

    link->v_r = 0;

    return link;
}

void link_destroy(link_t *link)
{
    if (!link) {
        return;
    }

    tpdu_ui_destroy(link->tpdu_ui);
    tpdu_destroy(link->tpdu);
    free(link);
}

int link_push_hdlc_frame(link_t *link, const hdlc_frame_t *hdlc_fr, tsdu_t **tsdu)
{
    if (!tsdu) {
        LOG(ERR, "link_push_hdlc_frame() tsdu == NULL");
        return -1;
    }
    *tsdu = NULL;

    if (hdlc_fr->command.cmd == COMMAND_INFORMATION) {
        LOG_IF(INFO) {
            LOG_("cmd=Information ");
            addr_print(&hdlc_fr->addr);
            LOGF(" n_r=%d n_s=%d P=%d\n",
                    hdlc_fr->command.information.n_r,
                    hdlc_fr->command.information.n_s,
                    hdlc_fr->command.information.p_e);
        }

        if (hdlc_fr->command.information.n_s != link->v_r) {
            LOG(INFO, "BROKEN LINK");
            link->v_r = hdlc_fr->command.information.n_s;
        }

        link->v_r = (link->v_r + 1) % 8;

        return tpdu_push_hdlc_frame(link->tpdu, hdlc_fr, tsdu);
    }

    if (hdlc_fr->command.cmd == COMMAND_SUPERVISION_RR ||
            hdlc_fr->command.cmd == COMMAND_SUPERVISION_RNR ||
            hdlc_fr->command.cmd == COMMAND_SUPERVISION_REJ) {
        LOG_IF(INFO) {
            switch(hdlc_fr->command.cmd) {
                case COMMAND_SUPERVISION_RR:
                    LOG_("cmd=RR ");
                    break;

                case COMMAND_SUPERVISION_RNR:
                    LOG_("cmd=RNR ");
                    break;

                case COMMAND_SUPERVISION_REJ:
                    LOG_("cmd=REJ ");
                    break;
            }
            addr_print(&hdlc_fr->addr);
            LOGF(" n_r=%d P=%d\n",
                    hdlc_fr->command.supervision.n_r,
                    hdlc_fr->command.supervision.p_e);
        }

        if (!cmpzero(hdlc_fr->data, hdlc_fr->nbits / 8)) {
            char buf[hdlc_fr->nbits / 8 * 3];
            LOG(WTF, "cmd=0x%02x, nonzero stuffing: %s\n",
                    hdlc_fr->command.cmd,
                    sprint_hex(buf, hdlc_fr->data, hdlc_fr->nbits / 8));
        }

        return 0;
    }

    if (hdlc_fr->command.cmd == COMMAND_UNNUMBERED_UI) {
        LOG_IF(DBG) {
            char buf[hdlc_fr->nbits / 8 * 3];
            LOG_("HDLC info=%s\n",
                    sprint_hex(buf, hdlc_fr->data, hdlc_fr->nbits / 8));
            LOGF("\t");
            addr_print(&hdlc_fr->addr);
            LOGF("\n");
        }
        return tpdu_ui_push_hdlc_frame(link->tpdu_ui, hdlc_fr, tsdu);
    }

    if (hdlc_fr->command.cmd == COMMAND_DACH) {
        LOG_IF(INFO) {
            LOG_("cmd ACK_DACH ");
            addr_print(&hdlc_fr->addr);
            LOGF("\n");
        }
        if (!cmpzero(hdlc_fr->data, hdlc_fr->nbits / 8)) {
            LOG_IF(WTF) {
                char buf[hdlc_fr->nbits / 8 * 3];
                LOG_("cmd: ACK_DACH, nonzero stuffing: %s\n",
                        sprint_hex(buf, hdlc_fr->data, hdlc_fr->nbits / 8));
            }
        }

        LOG(ERR, "TODO: ACK_DACH");
        // TODO: report ACK_DACH to application layer
        return 0;
    }

    if (hdlc_fr->command.cmd == COMMAND_UNNUMBERED_SNRM) {
        LOG_IF(INFO) {
            LOG_("cmd SNMR ");
            addr_print(&hdlc_fr->addr);
            LOGF("\n");
        }

        if (!cmpzero(hdlc_fr->data, hdlc_fr->nbits / 8)) {
            LOG_IF(WTF) {
                char buf[hdlc_fr->nbits / 8 * 3];
                LOG_("cmd: SNMR, nonzero stuffing: %s\n",
                        sprint_hex(buf, hdlc_fr->data, hdlc_fr->nbits / 8));
            }
        }

        LOG(ERR, "TODO: SNMR");
        // TODO: report SNMR to upper layer
        return 0;
    }

    if (hdlc_fr->command.cmd == COMMAND_UNNUMBERED_UI_VCH) {
        LOG_IF(DBG) {
            char buf[hdlc_fr->nbits / 8 * 3];
            LOG_("HDLC info=%s\n",
                    sprint_hex(buf, hdlc_fr->data, hdlc_fr->nbits / 8));
            LOGF("\t");
            addr_print(&hdlc_fr->addr);
            LOGF("\n");
        }

        lsdu_vch_t *lsdu;
        if (!lsdu_vch_decode_hdlc_frame(hdlc_fr, &lsdu)) {
            // LSDU should be send upwards like TSDU
            lsdu_vch_print(lsdu);
            lsdu_vch_destroy(lsdu);
        }

        return 0;
    }

    if (hdlc_fr->command.cmd == COMMAND_UNNUMBERED_UI_CD) {
        LOG_IF(DBG) {
            char buf[hdlc_fr->nbits / 8 * 3];
            LOG_("HDLC info=%s\n",
                    sprint_hex(buf, hdlc_fr->data, hdlc_fr->nbits / 8));
            LOGF("\t");
            addr_print(&hdlc_fr->addr);
            LOGF("\n");
        }

        lsdu_cd_t *lsdu;
        if (!lsdu_cd_decode(hdlc_fr->data, hdlc_fr->nbits / 8, &lsdu)) {
            // LSDU should be send upwards like TSDU
            lsdu_cd_print(lsdu);
            lsdu_cd_destroy(lsdu);
        }
        return 0;
    }

    LOG(INFO, "TODO CMD 0x%02x", hdlc_fr->command.cmd);

    return -1;
}

void link_tick(const timeval_t* tv, link_t *link)
{
    tpdu_du_tick(tv, link->tpdu_ui);
}
