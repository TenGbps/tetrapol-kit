#define LOG_PREFIX "link"

#include <tetrapol/link.h>
#include <tetrapol/log.h>
#include <tetrapol/lsdu_cd.h>
#include <tetrapol/misc.h>
#include <tetrapol/tpdu.h>
#include <tetrapol/lsdu_vch.h>

#include <stdlib.h>
#include <string.h>

struct link_priv_t {
    tpdu_t *tpdu;
    tpdu_ui_t *tpdu_ui;
    uint8_t v_r;    ///< v(r) PAS 0001-3-3 7.5.4.2.2
    uint8_t v_s;    ///< v(s) PAS 0001-3-3 7.5.4.2.2
    bool rx_glitch;
};

link_t *link_create(tpol_t *tpol, int log_ch)
{
    link_t *link = malloc(sizeof(link_t));
    if (!link) {
        return NULL;
    }

    link->tpdu_ui = tpdu_ui_create(tpol, FRAME_TYPE_DATA, log_ch);
    if (!link->tpdu_ui) {
        free(link);
        return NULL;
    }

    link->tpdu = tpdu_create(tpol, LOG_CH_SDCH);
    if (!link->tpdu) {
        tpdu_ui_destroy(link->tpdu_ui);
        free(link);
        return NULL;
    }

    link->v_r = 0;
    link->v_s = 0;
    link->rx_glitch = true;

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

int link_push_hdlc_frame(link_t *link, const hdlc_frame_t *hdlc_fr)
{
    if (hdlc_fr->command.cmd == COMMAND_INFORMATION) {
        LOG_IF(INFO) {
            char buf[ADDR_PRINT_BUF_SIZE];
            LOG(INFO, "cmd=Information %s n_r=%d n_s=%d P=%d",
                    addr_print(buf, &hdlc_fr->addr),
                    hdlc_fr->command.information.n_r,
                    hdlc_fr->command.information.n_s,
                    hdlc_fr->command.information.p_e);
        }

        if (hdlc_fr->command.information.n_s != link->v_r) {
            if (link->rx_glitch) {
                LOG(INFO, "link broken v_r=%d", link->v_r);
                tpdu_rx_glitch(link->tpdu);
                link->v_r = hdlc_fr->command.information.n_s;
                link->v_s = hdlc_fr->command.information.n_r;
            } else {
                // frame resend detected, we already have this one -> drop it
                LOG(INFO, "frame resend v_r=%d", link->v_r);
                return 0;
            }
        }
        link->rx_glitch = false;

        link->v_r = (link->v_r + 1) % 8;

        return tpdu_push_hdlc_frame(link->tpdu, hdlc_fr);
    }

    if (hdlc_fr->command.cmd == COMMAND_SUPERVISION_RR ||
            hdlc_fr->command.cmd == COMMAND_SUPERVISION_RNR ||
            hdlc_fr->command.cmd == COMMAND_SUPERVISION_REJ) {
        LOG_IF(INFO) {
            switch(hdlc_fr->command.cmd) {
                case COMMAND_SUPERVISION_RR:
                    LOG_("cmd=RR ");
                    link->v_s = hdlc_fr->command.supervision.n_r;
                    break;

                case COMMAND_SUPERVISION_RNR:
                    link->v_s = hdlc_fr->command.supervision.n_r;
                    LOG_("cmd=RNR ");
                    break;

                case COMMAND_SUPERVISION_REJ:
                    link->v_s = hdlc_fr->command.supervision.n_r;
                    LOG_("cmd=REJ ");
                    break;
            }
            char buf[ADDR_PRINT_BUF_SIZE];
            LOGF("%s n_r=%d P=%d\n",
                    addr_print(buf, &hdlc_fr->addr),
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
            char addr_buf[ADDR_PRINT_BUF_SIZE];
            LOG(DBG, "cmd=UI %s data=%s\n",
                    addr_print(addr_buf, &hdlc_fr->addr),
                    sprint_hex(buf, hdlc_fr->data, hdlc_fr->nbits / 8));
        }
        return tpdu_ui_push_hdlc_frame(link->tpdu_ui, hdlc_fr, NULL);
    }

    if (hdlc_fr->command.cmd == COMMAND_DACH) {
        LOG_IF(INFO) {
            char buf[ADDR_PRINT_BUF_SIZE];
            LOG(INFO, "cmd=ACK_DACH %s", addr_print(buf, &hdlc_fr->addr));
        }
        if (!cmpzero(hdlc_fr->data, hdlc_fr->nbits / 8)) {
            LOG_IF(WTF) {
                char buf[hdlc_fr->nbits / 8 * 3];
                LOG(WTF, "cmd=ACK_DACH, nonzero stuffing: %s",
                        sprint_hex(buf, hdlc_fr->data, hdlc_fr->nbits / 8));
            }
        }

        LOG(ERR, "TODO: ACK_DACH");
        // TODO: report ACK_DACH to application layer
        return 0;
    }

    if (hdlc_fr->command.cmd == COMMAND_UNNUMBERED_SNRM) {
        LOG_IF(INFO) {
            char buf[ADDR_PRINT_BUF_SIZE];
            LOG(INFO, "cmd=SNRM %s", addr_print(buf, &hdlc_fr->addr));
        }

        link->v_r = 0;
        link->v_s = 0;
// TODO: close all connections?

        if (!cmpzero(hdlc_fr->data, hdlc_fr->nbits / 8)) {
            LOG_IF(WTF) {
                char buf[hdlc_fr->nbits / 8 * 3];
                LOG(WTF, "cmd=SNRM, nonzero stuffing: %s",
                        sprint_hex(buf, hdlc_fr->data, hdlc_fr->nbits / 8));
            }
        }

        return 0;
    }

    if (hdlc_fr->command.cmd == COMMAND_UNNUMBERED_UI_VCH) {
        LOG_IF(DBG) {
            char buf[hdlc_fr->nbits / 8 * 3];
            char addr_buf[ADDR_PRINT_BUF_SIZE];
            LOG(DBG, "cmd=UI_VCH %s data=%s",
                    addr_print(addr_buf, &hdlc_fr->addr),
                    sprint_hex(buf, hdlc_fr->data, hdlc_fr->nbits / 8));
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
            char addr_buf[ADDR_PRINT_BUF_SIZE];
            LOG(DBG, "cmd=UI_CD %s data=%s",
                    addr_print(addr_buf, &hdlc_fr->addr),
                    sprint_hex(buf, hdlc_fr->data, hdlc_fr->nbits / 8));
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

void link_rx_glitch(link_t *link)
{
    link->rx_glitch = true;
}

void link_tick(time_evt_t* te, link_t *link)
{
    link->rx_glitch |= te->rx_glitch;
    tpdu_du_tick(te, link->tpdu_ui);
}
