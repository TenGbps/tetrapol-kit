#define LOG_PREFIX "lsdu_cd"

#include <tetrapol/bit_utils.h>
#include <tetrapol/log.h>
#include <tetrapol/lsdu_cd.h>
#include <tetrapol/misc.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static const char *codop_str[256] = {
    "N/A",                          // 0x00,
    "TP_ADDRESS",                   // 0x01,
    "TP_ALIAS",                     // 0x02,
    "AMBIENCE_LISTENING",           // 0x03,
    "TP_ADDRESS_CHIF",              // 0x04,
    "TP_ADDRES_INHIB",              // 0x05,
    "TP_ADDRESS_CHIF_INHIB",        // 0x06,
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
};

void lsdu_cd_destroy(lsdu_cd_t *lsdu)
{
    free(lsdu);
}

static int tp_address_decode(const uint8_t *data, int len, lsdu_cd_tp_address_t *lsdu)
{
    if (len != 11) {
        LOG(WTF, "invalid lenght %d", len);
        return -1;
    }

    lsdu->signature = get_bits(4, &data[1], 4);
    lsdu->modifier_number = get_bits(4, &data[1], 0);

    const uint8_t *d = &data[2];
    if (address_decode(&lsdu->rt_address, &d)) {
        LOG(WTF, "Only single address is supported");
    }
    lsdu->_stuffing_len = len - (d - data);
    memcpy(lsdu->_stuffing, d, lsdu->_stuffing_len);

    return 0;
}

int lsdu_cd_decode(const uint8_t *data, int len, lsdu_cd_t **lsdu)
{
    if (!lsdu) {
        LOG(ERR, "lsdu == NULL");
        return -1;
    }

    *lsdu = malloc(sizeof(lsdu_cd_t));
    if (!*lsdu) {
        return -1;
    }

    (*lsdu)->unknown.codop = data[0];

    switch ((*lsdu)->unknown.codop) {
        case TP_ADDRESS:
            return tp_address_decode(data, len, (lsdu_cd_tp_address_t *)*lsdu);

        default:
            LOG(INFO, "TODO LSDU_CD 0x%02x", (*lsdu)->unknown.codop);
            break;
    }

    memcpy(*lsdu, data, len);
    (*lsdu)->unknown.len = len;

    return 0;
}

static void tp_address_print(const lsdu_cd_tp_address_t *lsdu)
{
    LOGF("\tMODIFIER_NUMBER=%d\n", lsdu->modifier_number);
    LOGF("\tSIGNATURE=%d\n", lsdu->signature);
    address_print(&lsdu->rt_address);

    char buf[3*8];
    LOGF("\tSTUFFING=%s\n", sprint_hex(buf, lsdu->_stuffing, lsdu->_stuffing_len));
}

static void lsdu_cd_unknown_print(const lsdu_cd_t *lsdu)
{
    char buf[3 * sizeof(lsdu_cd_t)];
    LOGF("\tdata=%s\n",
            sprint_hex(buf, lsdu->unknown.data, lsdu->unknown.len));
}

void lsdu_cd_print(const lsdu_cd_t *lsdu)
{
    LOG_("CODOP=%d (%s)\n", lsdu->unknown.codop, codop_str[lsdu->unknown.codop]);

    switch (lsdu->unknown.codop) {
        case TP_ADDRESS:
            tp_address_print(&lsdu->tp_address);
            break;

        default:
            lsdu_cd_unknown_print(lsdu);
    }
}

