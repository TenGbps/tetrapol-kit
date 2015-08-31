#define LOG_PREFIX "link"

#include <tetrapol/link.h>
#include <tetrapol/log.h>
#include <tetrapol/misc.h>
#include <tetrapol/tpdu.h>

#include <stdlib.h>

struct link_priv_t {
    tpdu_t *tpdu;
    tpdu_ui_t *tpdu_ui;
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
        return tpdu_push_hdlc_frame(link->tpdu, hdlc_fr, tsdu);
    }

    if (hdlc_fr->command.cmd == COMMAND_SUPERVISION_RR ||
            hdlc_fr->command.cmd == COMMAND_SUPERVISION_RNR ||
            hdlc_fr->command.cmd == COMMAND_SUPERVISION_REJ) {
        LOG_IF(INFO) {
            switch(hdlc_fr->command.cmd) {
                case COMMAND_SUPERVISION_RR:
                    LOG_("\n\tcmd: RR\n\taddr=");
                    break;

                case COMMAND_SUPERVISION_RNR:
                    LOG_("\n\tcmd: RNR\n\taddr=");
                    break;

                case COMMAND_SUPERVISION_REJ:
                    LOG_("\n\tcmd: REJ\n\taddr=");
                    break;
            }
            addr_print(&hdlc_fr->addr);
            LOGF("\n\tn_r=%d P=%d\n",
                    hdlc_fr->command.supervision.recv_seq_no,
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
            LOG_("\n\tcmd ACK_DACH\n\taddr: ");
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
            LOG_("\n\tcmd SNMR\n\taddr: ");
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
        LOG(ERR, "TODO CMD UI_VCH");
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
        LOG(ERR, "TODO CMD UI_CD");
        return 0;
    }

    LOG(INFO, "TODO CMD 0x%02x", hdlc_fr->command.cmd);

    return -1;
}

void link_tick(const timeval_t* tv, link_t *link)
{
    tpdu_du_tick(tv, link->tpdu_ui);
}
