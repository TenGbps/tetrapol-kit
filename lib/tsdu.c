#define LOG_PREFIX "tsdu"

#include <tetrapol/log.h>
#include <tetrapol/tsdu.h>
#include <tetrapol/misc.h>
#include <tetrapol/bit_utils.h>
#include <tetrapol/misc.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_LEN(len, min_len, tsdu) \
    if ((len) < (min_len)) { \
        LOG(ERR, "data too short %d < %d", (len), (min_len)); \
        tsdu_destroy((tsdu_base_t *)tsdu); \
        return NULL; \
    }

const int CELL_RADIO_PARAM_PWR_TX_ADJUST_TO_DBM[16] = {
    -76, -72, -68, -64, -60, -56, -52, -48,
    -44, -40, -36, -32, -28, -24, -20, -16,
};

const int CELL_RADIO_PARAM_RX_LEV_ACCESS_TO_DBM[16] = {
    -92, -88, -84, -80, -76, -72, -68, -64,
    -60, -56, -52, -48, -44, -40, -36, -32,
};

static void tsdu_base_set_nopts(tsdu_base_t *tsdu, int noptionals)
{
    tsdu->noptionals = noptionals;
    memset(tsdu->optionals, 0, sizeof(void *[noptionals]));
}

#define tsdu_create(TSDU_TYPE, noptionals) \
    (TSDU_TYPE *) tsdu_create_(sizeof(TSDU_TYPE), noptionals)

static tsdu_t *tsdu_create_(int size, int noptionals)
{
    tsdu_t *tsdu = malloc(size);
    if (!tsdu) {
        return NULL;
    }
    tsdu_base_set_nopts(tsdu, noptionals);
    return tsdu;
}

void tsdu_destroy(tsdu_base_t *tsdu)
{
    if (!tsdu) {
        return;
    }
    for (int i = 0; i < tsdu->noptionals; ++i) {
        free(tsdu->optionals[i]);
    }
    free(tsdu);
}

static void activation_mode_decode(activation_mode_t *am, uint8_t data)
{
    am->hook = get_bits(2, &data, 0);
    am->type = get_bits(2, &data, 2);
}

static void cell_id_decode1(cell_id_t *cell_id, const uint8_t *data)
{
    int type = get_bits(2, data, 0);
    if (type == CELL_ID_FORMAT_0) {
        cell_id->bs_id = get_bits(6, data, 2);
        cell_id->rsw_id = get_bits(4, data, 8);
    } else if (type == CELL_ID_FORMAT_1) {
        cell_id->bs_id = get_bits(4, data, 8);
        cell_id->rsw_id = get_bits(6, data, 2);
    } else {
        LOG(WTF, "unknown cell_id_type (%d)", type);
        cell_id->bs_id = -1;
        cell_id->rsw_id = -1;
    }
}

// specific for d_system_info - cell in offline mode
static void cell_id_decode2(cell_id_t *cell_id, const uint8_t *data)
{
    int type = get_bits(2, data, 8);
    if (type == CELL_ID_FORMAT_0) {
        cell_id->bs_id = get_bits(6, data, 10);
        cell_id->rsw_id = get_bits(4, data, 4);;
    } else if (type == CELL_ID_FORMAT_1) {
        cell_id->bs_id = get_bits(4, data, 4);;
        cell_id->rsw_id = get_bits(6, data, 10);;
    } else {
        LOG(WTF, "unknown cell_id_type (%d)", type);
        cell_id->bs_id = -1;
        cell_id->rsw_id = -1;
    }
}

static tsdu_d_authentication_t *
d_authentication_decode(const uint8_t *data, int len)
{
    tsdu_d_authentication_t *tsdu = tsdu_create(tsdu_d_authentication_t, 0);
    if (!tsdu) {
        return NULL;
    }
    CHECK_LEN(len, 16, tsdu);

    tsdu->key_reference._data = data[1];
    memcpy(tsdu->valid_rt, &data[2], sizeof(tsdu->valid_rt));

    return tsdu;
}

static tsdu_d_crisis_notification_t *d_crisis_notification_decode(
        const uint8_t *data, int len)
{
    CHECK_LEN(len, 10, NULL);

    tsdu_d_crisis_notification_t *tsdu = tsdu_create(
            tsdu_d_crisis_notification_t, 0);
    if (!tsdu) {
        return NULL;
    }

    const uint8_t *data_ = &data[1];
    if (address_decode(&tsdu->calling_adr, &data_)) {
        LOG(WTF, "Only single addres in list is supported.");
    }
    tsdu->organisation = data[6];
    tsdu->coverage_id = data[7];
    if (data[8]) {
        LOG(WTF, "Crisis - nonzero undocumented field=0x%02x", data[8]);
    }
    tsdu->og_nb = get_bits(4, &data[9], 0);
    if (tsdu->og_nb > 5) {
        LOG(WTF, "Too large OG_NB %d", tsdu->og_nb);
        tsdu_destroy(&tsdu->base);
        return NULL;
    }
    CHECK_LEN(len, 10 + (12 * tsdu->og_nb) / 12, tsdu);
    for (int i = 0; i < tsdu->og_nb; ++i) {
        tsdu->group_ids[i] = get_bits(12, &data[9], 4 + 12*i);
    }

    return tsdu;
}

static tsdu_d_group_reject_t *d_group_reject_decode(const uint8_t *data, int len)
{
    CHECK_LEN(len, 6, NULL);

    tsdu_d_group_reject_t *tsdu = tsdu_create(tsdu_d_group_reject_t, 0);
    if (!tsdu) {
        return NULL;
    }

    activation_mode_decode(&tsdu->activation_mode, data[1]);
    tsdu->group_id  = get_bits(12, data + 1, 4);
    tsdu->coverage_id = data[3];
    tsdu->_zero = data[4];
    if (tsdu->_zero) {
        LOG(WTF, "Nonzero zero=0x%02x", tsdu->_zero);
    }
    tsdu->cause = data[5];

    return tsdu;
}

static tsdu_d_cch_open_t *d_cch_open_decode(const uint8_t *data, int len)
{
    if (len != 1) {
        LOG(WTF, "Invalid len 1 != %d", len);
        return NULL;
    }
    return tsdu_create(tsdu_d_cch_open_t, 0);
}

static tsdu_d_refusal_t *d_refusal_decode(const uint8_t *data, int len)
{
    CHECK_LEN(len, 2, NULL);

    tsdu_d_refusal_t *tsdu = tsdu_create(tsdu_d_refusal_t, 0);
    if (!tsdu) {
        return NULL;
    }
    tsdu->cause = data[1];

    return tsdu;
}

static tsdu_d_reject_t *d_reject_decode(const uint8_t *data, int len)
{
    CHECK_LEN(len, 2, NULL);

    tsdu_d_reject_t *tsdu = tsdu_create(tsdu_d_reject_t, 0);
    if (!tsdu) {
        return NULL;
    }
    tsdu->cause = data[1];

    return tsdu;
}

static tsdu_d_call_alert_t *d_call_alert_decode(const uint8_t *data, int len)
{
    return tsdu_create(tsdu_d_call_alert_t, 0);
}

static tsdu_d_hook_on_invitation_t *d_hook_on_invitation_decode(
        const uint8_t *data, int len)
{
    CHECK_LEN(len, 2, NULL);

    tsdu_d_hook_on_invitation_t *tsdu = tsdu_create(tsdu_d_hook_on_invitation_t, 0);
    if (!tsdu) {
        return NULL;
    }
    tsdu->cause = data[1];

    return tsdu;
}

static tsdu_d_release_t *d_release_decode(const uint8_t *data, int len)
{
    CHECK_LEN(len, 2, NULL);

    tsdu_d_release_t *tsdu = tsdu_create(tsdu_d_release_t, 0);
    if (!tsdu) {
        return NULL;
    }
    tsdu->cause = data[1];

    return tsdu;
}

static tsdu_d_additional_participants_t *d_additional_participants_decode(
        const uint8_t *data, int len)
{
    CHECK_LEN(len, 7, NULL);

    tsdu_d_additional_participants_t *tsdu = tsdu_create(
            tsdu_d_additional_participants_t, 1);
    if (!tsdu) {
        return NULL;
    }

    tsdu->coverage_id = data[1];
    if (address_list_decode(&tsdu->calling_adr, &data[2]) == -1) {
        tsdu_destroy(&tsdu->base);
        return NULL;
    }

    if (len >= 8) {
        if (address_list_decode(&tsdu->calling_adr, &data[7]) == -1) {
            tsdu_destroy(&tsdu->base);
            return NULL;
        }
    }

    if (len >= 13) {
        if (address_list_decode(&tsdu->calling_adr, &data[12]) == -1) {
            tsdu_destroy(&tsdu->base);
            return NULL;
        }
    }

    return tsdu;
}

static tsdu_d_call_setup_t *d_call_setup_decode(const uint8_t *data, int len)
{
    CHECK_LEN(len, 6, NULL);

    tsdu_d_call_setup_t *tsdu = tsdu_create(tsdu_d_call_setup_t, 0);
    if (!tsdu) {
        return NULL;
    }

    const uint8_t *adr_data = &data[1];
    if (address_decode(&tsdu->calling_adr, &adr_data)) {
        LOG(ERR, "Only single address is supported in calling_adr");
    }

    tsdu->has_add_setup_param = false;
    if (len >= 8) {
        tsdu->has_add_setup_param = (data[6] == IEI_ADD_SETUP_PARAM);
        if (!tsdu->has_add_setup_param) {
            LOG(WTF, "Unexpected IEI 0x%02x", data[6]);
            return tsdu;
        }
        tsdu->add_setup_param._data = data[7];
    }

    return tsdu;
}

static tsdu_d_ability_mngt_t *d_ability_mngt_decode(const uint8_t *data, int len)
{
    if (len - 1 > SIZEOF(tsdu_d_ability_mngt_t, data)) {
        LOG(WTF, "Message too long %d", len - 1);
        return NULL;
    }

    tsdu_d_ability_mngt_t *tsdu = tsdu_create(tsdu_d_ability_mngt_t, 0);
    if (!tsdu) {
        return NULL;
    }

    tsdu->data_len = len - 1;
    memcpy(tsdu->data, &data[1], len - 1);

    return tsdu;
}

static tsdu_d_dch_open_t *d_dch_open_decode(const uint8_t *data, int len)
{
    if (len != 1) {
        LOG(WTF, "Invalid len 1 != %d", len);
        return NULL;
    }

    return tsdu_create(tsdu_d_dch_open_t, 0);
}

static tsdu_d_data_request_t *d_data_request_decode(const uint8_t *data, int len)
{
    CHECK_LEN(len, 16, NULL);

    tsdu_d_data_request_t *tsdu = tsdu_create(tsdu_d_data_request_t, 0);
    if (!tsdu) {
        return NULL;
    }

    tsdu->key_reference_auth._data = data[1];
    memcpy(tsdu->valid_rt, &data[2], sizeof(tsdu->valid_rt));
    tsdu->key_reference_ciph._data = data[10];
    tsdu->trans_mode =      get_bits(4, &data[11], 4);
    tsdu->trans_param1 =    get_bits(16, &data[12], 0);
    tsdu->trans_param2 =    get_bits(16, &data[14], 0);
    tsdu->has_trans_param3 = (tsdu->trans_mode == TRANS_MODE_UDP_MSG);
    if (tsdu->has_trans_param3) {
        CHECK_LEN(len, 18, tsdu);
        tsdu->trans_param3 = get_bits(16, &data[16], 0);
    }
    return tsdu;
}

static tsdu_d_connect_cch_t *d_connect_cch_decode(const uint8_t *data, int len)
{
    if (len != 1) {
        LOG(WTF, "Invalid len 1 != %d", len);
        return NULL;
    }

    return tsdu_create(tsdu_d_connect_cch_t, 0);
}

static tsdu_d_data_authentication_t *d_data_authentication_decode(
        const uint8_t *data, int len)
{
    CHECK_LEN(len, 11, NULL);

    tsdu_d_data_authentication_t *tsdu = tsdu_create(tsdu_d_data_authentication_t, 0);
    if (!tsdu) {
        return NULL;
    }

    tsdu->key_reference_auth._data = data[1];
    memcpy(tsdu->valid_rt, &data[2], sizeof(tsdu->valid_rt));
    tsdu->key_reference_ciph._data = data[10];
    return tsdu;
}

static tsdu_d_data_msg_down_t *d_data_msg_down_decode(const uint8_t *data, int len)
{
    if (len - 1 > SIZEOF(tsdu_d_data_msg_down_t, data)) {
        LOG(WTF, "Message too large %d > %d",
                len - 1, (int)SIZEOF(tsdu_d_data_msg_down_t, data));
        return NULL;
    }

    tsdu_d_data_msg_down_t *tsdu = tsdu_create(tsdu_d_data_msg_down_t, 0);
    if (!tsdu) {
        return NULL;
    }

    tsdu->data_len = len - 1;
    memcpy(tsdu->data, &data[1], tsdu->data_len);

    return tsdu;
}

static tsdu_d_authorisation_t *
d_authorisation_decode(const uint8_t *data, int len)
{
    tsdu_d_authorisation_t *tsdu = tsdu_create(tsdu_d_authorisation_t, 0);
    if (!tsdu) {
        return NULL;
    }
    CHECK_LEN(len, 8, tsdu);

    tsdu->has_key_reference = (data[1] == IEI_KEY_REFERENCE);
    if (tsdu->has_key_reference) {
        tsdu->key_reference._data = data[2];
    }

    return tsdu;
}

static tsdu_d_group_paging_t *d_group_paging_decode(const uint8_t *data, int len)
{
    tsdu_d_group_paging_t *tsdu = tsdu_create(tsdu_d_group_paging_t, 0);
    if (!tsdu) {
        return NULL;
    }
    CHECK_LEN(len, 5, tsdu);

    activation_mode_decode(&tsdu->activation_mode, data[1]);
    tsdu->group_id              = get_bits(12, &data[1], 4);
    tsdu->coverage_id           = get_bits(8,  &data[3], 0);
    tsdu->key_reference._data   = get_bits(8,  &data[4], 0);

    return tsdu;
}

static tsdu_d_forced_registration_t *
d_forced_registration_decode(const uint8_t *data, int len)
{
    tsdu_d_forced_registration_t *tsdu = tsdu_create(tsdu_d_forced_registration_t, 0);
    if (!tsdu) {
        return NULL;
    }
    CHECK_LEN(len, 9, tsdu);

    const uint8_t *adr_data = &data[1];
    if (address_decode(&tsdu->calling_adr, &adr_data)) {
        LOG(ERR, "Only single address is supported in calling_adr");
    }
    return tsdu;
}

static tsdu_d_group_activation_t *
d_group_activation_decode(const uint8_t *data, int len)
{
    tsdu_d_group_activation_t *tsdu = tsdu_create(tsdu_d_group_activation_t, 0);
    if (!tsdu) {
        return NULL;
    }

    CHECK_LEN(len, 9, tsdu);

    int _zero0;
    activation_mode_decode(&tsdu->activation_mode, data[1]);
    tsdu->group_id              = get_bits(12, data + 1, 4);
    tsdu->coverage_id           = get_bits(8,  data + 3, 0);
    _zero0                      = get_bits(4,  data + 4, 0);
    tsdu->channel_id            = get_bits(12, data + 4, 4);
    tsdu->u_ch_scrambling       = get_bits(8,  data + 6, 0);
    tsdu->d_ch_scrambling       = get_bits(8,  data + 7, 0);
    tsdu->key_reference._data   = get_bits(8,  data + 8, 0);

    if (_zero0 != 0) {
        LOG(WTF, "nonzero padding: 0x%02x", _zero0);
    }

    tsdu->has_addr_tti = false;
    if (len >= 12) {
        // FIXME: proper IEI handling
        uint8_t iei = get_bits(8, data + 9, 0);
        if (iei != IEI_TTI) {
            LOG(WTF, "expected IEI_TTI got %d", iei);
        } else {
            tsdu->has_addr_tti = true;
            addr_parse(&tsdu->addr_tti, &data[10], 0);
        }
    }

    if (len > 12) {
        LOG(WTF, "unused bytes (%d)", len);
    }

    return tsdu;
}

static tsdu_d_group_list_t *d_group_list_decode(const uint8_t *data, int len)
{
    tsdu_d_group_list_t *tsdu = tsdu_create(tsdu_d_group_list_t, 3);
    if (!tsdu) {
        return NULL;
    }

    tsdu->nemergency = 0;
    tsdu->ngroup = 0;
    tsdu->nopen = 0;

    int rlen = 2; ///< required data length
    CHECK_LEN(len, rlen, tsdu);
    tsdu->reference_list._data = get_bits(8, data + 1, 0);
    if (tsdu->reference_list.revision == 0) {
        return tsdu;
    }

    rlen += 1;
    CHECK_LEN(len, rlen, tsdu);
    tsdu->index_list._data = get_bits(8, data + 2, 0);
    data += 3;
    do {
        rlen += 1;
        CHECK_LEN(len, rlen, tsdu);
        const type_nb_t type_nb = {
            ._data = get_bits(8, data, 0),
        };
        if (type_nb.type == TYPE_NB_TYPE_END) {
            break;
        }
        data += 1;

        if (type_nb.type == TYPE_NB_TYPE_EMERGENCY) {
            const int n = tsdu->nemergency + type_nb.number;
            const int l = sizeof(tsdu_d_group_list_emergency_t[n]);
            tsdu_d_group_list_emergency_t *p = realloc(tsdu->emergency, l);
            if (!p) {
                tsdu_destroy(&tsdu->base);
                return NULL;
            }
            tsdu->emergency = p;
            for ( ; tsdu->nemergency < n; ++tsdu->nemergency) {
                rlen += 2;
                if (rlen > len) {
                    break;
                }
                const int i = tsdu->nemergency;
                cell_id_decode1(&tsdu->emergency[i].cell_id, data);
                int zero = get_bits(4, data + 1, 4);
                if (zero != 0) {
                    LOG(WTF, "nonzero padding (%d)", zero);
                }
                data += 2;
            }
        }

        if (type_nb.type == TYPE_NB_TYPE_OPEN) {
            const int n = tsdu->nopen + type_nb.number;
            const int l = sizeof(tsdu_d_group_list_open_t[n]);
            tsdu_d_group_list_open_t *p = realloc(tsdu->open, l);
            if (!p) {
                tsdu_destroy(&tsdu->base);
                return NULL;
            }
            tsdu->open = p;
            for ( ; tsdu->nopen < n; ++tsdu->nopen) {
                rlen += 5;
                if (rlen > len) {
                    break;
                }
                const int i = tsdu->nopen;
                tsdu->open[i].coverage_id           = get_bits(8, data, 0);
                tsdu->open[i].call_priority         = get_bits(4, data + 1, 0);
                tsdu->open[i].group_id              = get_bits(12, data + 1, 4);
                uint8_t padding                     = get_bits(2, data + 3, 0);
                if (padding != 0) {
                    LOG(WTF, "nonzero padding (%d)", padding);
                }
                tsdu->open[i].och_parameters.add    = get_bits(1, data + 3, 2);
                tsdu->open[i].och_parameters.mbn    = get_bits(1, data + 3, 3);
                tsdu->open[i].neighbouring_cell     = get_bits(12, data + 3, 4);
                data += 5;
            }
        }
        if (type_nb.type == TYPE_NB_TYPE_TALK_GROUP) {
            const int n = tsdu->ngroup + type_nb.number;
            const int l = sizeof(tsdu_d_group_list_talk_group_t[n]);
            tsdu_d_group_list_talk_group_t *p = realloc(tsdu->group, l);
            if (!p) {
                tsdu_destroy(&tsdu->base);
                return NULL;
            }
            tsdu->group = p;
            for ( ; tsdu->ngroup < n; ++tsdu->ngroup) {
                rlen += 4;
                if (rlen > len) {
                    break;
                }
                const int i = tsdu->ngroup;
                tsdu->group[i].coverage_id          = get_bits(8, data, 0);
                uint8_t zero                        = get_bits(8, data + 1, 0);
                if (zero != 0) {
                    LOG(WTF, "nonzero padding in talk group-1 (%d)", zero);
                }
                uint8_t padding                     = get_bits(4, data + 2, 0);
                if (padding != 0) {
                    LOG(WTF, "nonzero padding in talk group-2 (%d)", padding);
                }
                tsdu->group[i].neighbouring_cell    = get_bits(12, data + 2, 4);
                data += 4;
            }
        }
    } while(true);

    return tsdu;
}

static tsdu_d_group_composition_t *d_group_composition_decode(
        const uint8_t *data, int len)
{
    tsdu_d_group_composition_t *tsdu = tsdu_create(tsdu_d_group_composition_t, 0);
    if (!tsdu) {
        return NULL;
    }

    CHECK_LEN(len, 3, tsdu);

    tsdu->group_id = get_bits(12, data + 1, 0);
    tsdu->og_nb = get_bits(4, data + 2, 4);

    CHECK_LEN(len, 3 + (12*tsdu->og_nb + 7) / 8, tsdu);

    int skip = 0;
    for (int i = 0; i < tsdu->og_nb; ++i) {
        tsdu->group_ids[i] = get_bits(12, data + 3, skip);
        skip += 12;
    }

    return tsdu;
}

static cell_id_list_t *iei_cell_id_list_decode(cell_id_list_t *cell_ids,
        const uint8_t *data, int len)
{
    int n = cell_ids ? cell_ids->len : 0;
    n += len / 2;
    cell_id_list_t *p = realloc(cell_ids,
            sizeof(cell_id_list_t) + sizeof(cell_id_t[n]));
    if (!p) {
        LOG(ERR, "ERR OOM");
        return NULL;
    }
    if (!cell_ids) {
        p->len = 0;
    }
    cell_ids = p;

    for ( ; cell_ids->len < n; ++cell_ids->len) {
        cell_id_decode1(&cell_ids->cell_ids[cell_ids->len], data);
        data += 2;
    }

    return cell_ids;
}

cell_bn_list_t *iei_cell_bn_list_decode(
        cell_bn_list_t *cell_bns, const uint8_t *data, int len)
{
    int n = cell_bns ? cell_bns->len : 0;
    n +=  len * 2 / 3;
    cell_bn_list_t *p = realloc(cell_bns,
            sizeof(cell_bn_list_t) + sizeof(cell_bn_t[n]));
    if (!p) {
        LOG(ERR, "ERR OOM");
        return NULL;
    }
    if (!cell_bns) {
        p->len = 0;
    }
    cell_bns = p;

    for (int offs = 0; cell_bns->len < n; ++cell_bns->len) {
        cell_bns->cell_bn[cell_bns->len]._data = get_bits(12, data, offs);
        offs += 12;
    }

    return cell_bns;
}

static tsdu_d_neighbouring_cell_t *d_neighbouring_cell_decode(const uint8_t *data, int len)
{
    tsdu_d_neighbouring_cell_t *tsdu = tsdu_create(tsdu_d_neighbouring_cell_t, 2);
    if (!tsdu) {
        return NULL;
    }
    CHECK_LEN(len, 2, tsdu);

    uint8_t _zero                               = get_bits(4, data + 1, 0);
    tsdu->ccr_config.number                     = get_bits(4, data + 1, 4);
    if (_zero != 0) {
        LOG(WTF, "d_neighbouring_cell padding != 0 (%d)", _zero);
    }

    if (!tsdu->ccr_config.number) {
        return tsdu;
    }

    tsdu->ccr_param = data[2];
    if (tsdu->ccr_param) {
        LOG(WTF, "d_neighbouring_cell ccr_param != 0 (%d)", tsdu->ccr_param);
    }

    data += 3;
    len -= 3;
    CHECK_LEN(len, 3 * tsdu->ccr_config.number, tsdu);
    for (int i = 0; i < tsdu->ccr_config.number; ++i) {
        tsdu->adj_cells[i].bn_nb                = get_bits(4,  data, 0);
        tsdu->adj_cells[i].channel_id           = get_bits(12, data, 4);
        tsdu->adj_cells[i].adjacent_param._data = get_bits(8,  data + 2, 0);
        if (tsdu->adj_cells[i].adjacent_param._reserved) {
            LOG(WTF, "adjacent_param._reserved != 0");
        }
        data += 3;
        len -= 3;
    }

    while (len > 0) {
        CHECK_LEN(len, 2, tsdu);
        const uint8_t iei                       = get_bits(8, data, 0);
        const uint8_t ie_len                    = get_bits(8, data + 1, 0);
        data += 2;
        len -= 2;
        CHECK_LEN(len, ie_len, tsdu);
        if (iei == IEI_CELL_ID_LIST && ie_len) {
            cell_id_list_t *p = iei_cell_id_list_decode(
                    tsdu->cell_ids, data, ie_len);
            if (!p) {
                break;
            }
            tsdu->cell_ids = p;
        } else if (iei == IEI_ADJACENT_BN_LIST && ie_len) {
            cell_bn_list_t *p = iei_cell_bn_list_decode(
                    tsdu->cell_bns, data, ie_len);
            if (!p) {
                break;
            }
            tsdu->cell_bns = p;
        } else {
            if (ie_len) {
                LOG(WTF, "d_neighbouring_cell unknown iei (0x%x)", iei);
            }
        }
        data += ie_len;
        len -= ie_len;
    }

    return tsdu;
}

static tsdu_d_system_info_t *d_system_info_decode(const uint8_t *data, int len)
{
    tsdu_d_system_info_t *tsdu = tsdu_create(tsdu_d_system_info_t, 0);
    if (!tsdu) {
        return NULL;
    }

    // minimal size of disconnected mode
    CHECK_LEN(len, 9, tsdu);

    tsdu->cell_state._data = get_bits(8, data + 1, 0);
    switch (tsdu->cell_state.mode) {
        case CELL_STATE_MODE_NORMAL:
            CHECK_LEN(len, 17, tsdu);
            tsdu->cell_config._data                     = get_bits( 8, data + 2, 0);
            tsdu->country_code                          = get_bits( 8, data + 3, 0);
            tsdu->system_id._data                       = get_bits( 8, data + 4, 0);
            tsdu->loc_area_id._data                     = get_bits( 8, data + 5, 0);
            tsdu->bn_id                                 = get_bits( 8, data + 6, 0);
            cell_id_decode1(&tsdu->cell_id, data + 7);
            tsdu->cell_bn._data                         = get_bits(12, data + 8, 4);
            tsdu->u_ch_scrambling                       = get_bits( 8, data + 10, 0);
            tsdu->cell_radio_param.tx_max               = get_bits( 3, data + 11, 0);
            tsdu->cell_radio_param.radio_link_timeout   = get_bits( 5, data + 11, 3);
            tsdu->cell_radio_param.pwr_tx_adjust        = get_bits( 4, data + 12, 0);
            tsdu->cell_radio_param.rx_lev_access        = get_bits( 4, data + 12, 4);
            tsdu->system_time                           = get_bits( 8, data + 13, 0);
            tsdu->cell_access._data                     = get_bits( 8, data + 14, 0);
            tsdu->_unused_1                             = get_bits( 4, data + 15, 0);
            tsdu->superframe_cpt                        = get_bits(12, data + 15, 4);
            break;

        default:
            LOG(WTF, "unknown cell_state.mode=%d", tsdu->cell_state.mode);

        case CELL_STATE_MODE_DISC_INTERN_BN:
        case CELL_STATE_MODE_DISC_MAIN_SWITCH:
        case CELL_STATE_MODE_DISC_RADIOSWITCH:
        case CELL_STATE_MODE_DISC_BSC:
            tsdu->cell_state._data &= 0xf0;
            cell_id_decode2(&tsdu->cell_id, data + 1);
            tsdu->bn_id                                 = get_bits( 8, data + 3, 0);
            tsdu->u_ch_scrambling                       = get_bits( 8, data + 4, 0);
            tsdu->cell_radio_param.tx_max               = get_bits( 3, data + 5, 0);
            tsdu->cell_radio_param.radio_link_timeout   = get_bits( 5, data + 5, 3);
            tsdu->cell_radio_param.pwr_tx_adjust        = get_bits( 4, data + 6, 0);
            tsdu->cell_radio_param.rx_lev_access        = get_bits( 4, data + 6, 4);
            tsdu->band                                  = get_bits( 4, data + 7, 0);
            tsdu->channel_id                            = get_bits(12, data + 7, 4);
            break;
    }

    return tsdu;
}

static tsdu_d_registration_nak_t *d_registration_nak_decode(const uint8_t *data, int len)
{
    tsdu_d_registration_nak_t *tsdu = tsdu_create(tsdu_d_registration_nak_t, 0);
    if (!tsdu) {
        return NULL;
    }

    CHECK_LEN(len, 10, tsdu);

    tsdu->cause                 = data[1];
    const uint8_t *adr_data = &data[2];
    if (address_decode(&tsdu->host_adr, &adr_data)) {
        LOG(ERR, "Only single address NAK is supported");
    }
    tsdu->bn_id                 = data[7];
    cell_id_decode1(&tsdu->cell_id, data + 8);

    return tsdu;
}

static tsdu_d_registration_ack_t *d_registration_ack_decode(const uint8_t *data, int len)
{
    tsdu_d_registration_ack_t *tsdu = tsdu_create(tsdu_d_registration_ack_t, 0);
    if (!tsdu) {
        return NULL;
    }

    CHECK_LEN(len, 14, tsdu);

    tsdu->complete_reg          = data[1];
    tsdu->rt_min_activity       = data[2];
    tsdu->rt_status._data       = data[3];
    const uint8_t *adr_data = &data[4];
    if (address_decode(&tsdu->host_adr, &adr_data)) {
        LOG(ERR, "Only single address ACK is supported");
    }
    tsdu->rt_min_registration   = data[9];
    tsdu->tlr_value             = data[10];
    tsdu->rt_data_info._data    = data[11];
    tsdu->group_id              = get_bits(12, data + 12, 0);

    tsdu->has_coverage_id = false;
    if (len >= 16) {
        switch(data[14]) {
            case IEI_COVERAGE_ID:
                tsdu->has_coverage_id = true;
                tsdu->coverage_id = data[15];
                break;

            default:
                LOG(WTF, "Unexpected IEI 0x%x", data[14]);
        };
    }

    return tsdu;
}

static tsdu_d_connect_dch_t *d_connect_dch_decode(const uint8_t *data, int len)
{
    tsdu_d_connect_dch_t *tsdu = tsdu_create(tsdu_d_connect_dch_t, 0);
    if (!tsdu) {
        return NULL;
    }

    CHECK_LEN(len, 6, tsdu);

    tsdu->dch_low_layer    = data[1];
    tsdu->channel_id       = get_bits(12, &data[2], 4);
    tsdu->u_ch_scrambling  = data[4];
    tsdu->d_ch_scrambling  = data[5];

    return tsdu;
}

static tsdu_d_return_t *d_return_decode(const uint8_t *data, int len)
{
    tsdu_d_return_t *tsdu = tsdu_create(tsdu_d_return_t, 0);
    if (!tsdu) {
        return NULL;
    }

    CHECK_LEN(len, 2, tsdu);

    tsdu->cause = data[1];

    return tsdu;
}

static tsdu_d_group_idle_t *d_group_idle_decode(const uint8_t *data, int len)
{
    tsdu_d_group_idle_t *tsdu = tsdu_create(tsdu_d_group_idle_t, 0);
    if (!tsdu) {
        return NULL;
    }

    CHECK_LEN(len, 2, tsdu);

    tsdu->cause = data[1];

    return tsdu;
}

static tsdu_d_location_activity_ack_t *d_location_activity_ack_decode(
        const uint8_t *data, int len)
{
    CHECK_LEN(len, 2, NULL);

    tsdu_d_location_activity_ack_t *tsdu =
        tsdu_create(tsdu_d_location_activity_ack_t, 0);
    if (!tsdu) {
        return NULL;
    }

    tsdu->rt_status._data = data[1];

    return tsdu;
}

static tsdu_d_ech_overload_id_t *d_ech_overload_id_decode(const uint8_t *data, int len)
{
    tsdu_d_ech_overload_id_t *tsdu = tsdu_create(tsdu_d_ech_overload_id_t, 0);
    if (!tsdu) {
        return NULL;
    }

    CHECK_LEN(len, 6, tsdu);

    tsdu->activation_mode.hook = get_bits(2, data + 1, 0);
    tsdu->activation_mode.type = get_bits(2, data + 1, 2);
    tsdu->group_id = get_bits(12, data + 1, 4);
    cell_id_decode1(&tsdu->cell_id, data + 3);
    tsdu->organisation = get_bits(8, data + 5, 0);

    return tsdu;
}

static tsdu_unknown_codop_t *d_unknown_parse(const uint8_t *data, int len)
{
    tsdu_unknown_codop_t *tsdu = tsdu_create(tsdu_unknown_codop_t, 1);
    if (!tsdu) {
        return NULL;
    }

    tsdu->len = len;
    if (!len) {
        return tsdu;
    }

    tsdu->data = malloc(len);
    if (!tsdu->data) {
        tsdu_destroy(&tsdu->base);
        return NULL;
    }

    memcpy(tsdu->data, data, len);

    return tsdu;
}

static tsdu_d_data_end_t *d_data_end_decode(const uint8_t *data, int len)
{
    tsdu_d_data_end_t *tsdu = tsdu_create(tsdu_d_data_end_t, 0);
    if (!tsdu) {
        return NULL;
    }

    CHECK_LEN(len, 2, tsdu);
    tsdu->cause = data[1];

    return tsdu;
}

static tsdu_d_datagram_notify_t *d_datagram_notify_decode(const uint8_t *data, int len)
{
    tsdu_d_datagram_notify_t *tsdu = tsdu_create(tsdu_d_datagram_notify_t, 0);
    if (!tsdu) {
        return NULL;
    }

    CHECK_LEN(len, 5, tsdu);

    tsdu->call_priority         = get_bits(4, data + 1, 4);
    tsdu->message_reference     = data[2] | (data[3] << 8);
    tsdu->key_reference._data   = data[4];

    if (len >= 7) {
        tsdu->destination_port  = data[5] | (data[6] << 8);
    } else {
        tsdu->destination_port = -1;
    }

    return tsdu;
}

static tsdu_d_datagram_t *d_datagram_decode(const uint8_t *data, int len)
{
    if (len < 5) {
        LOG(WTF, "too short");
        return NULL;
    }
    len -= 5;

    tsdu_d_datagram_t *tsdu = malloc(sizeof(tsdu_d_datagram_t) + len);
    if (!tsdu) {
        return NULL;
    }

    tsdu_base_set_nopts(&tsdu->base, 0);

    tsdu->call_priority = get_bits(4, data + 1, 4);
    tsdu->message_reference = data[2] | (data[3] << 8);
    tsdu->key_reference._data = data[4];
    tsdu->len = len;
    memcpy(tsdu->data, data + 5, len);

    return tsdu;
}

static tsdu_d_explicit_short_data_t *d_explicit_short_data_decode(
        const uint8_t *data, int len)
{
    if (len < 1) {
        LOG(WTF, "too short");
        return NULL;
    }
    len -= 1;

    tsdu_d_explicit_short_data_t *tsdu = malloc(
            sizeof(tsdu_d_explicit_short_data_t) + len);
    if (!tsdu) {
        LOG(ERR, "ERR OOM");
        return NULL;
    }
    tsdu_base_set_nopts(&tsdu->base, 0);

    tsdu->len = len;
    memcpy(tsdu->data, data + 1, len);

    return tsdu;
}

static tsdu_d_call_start_t *d_call_start_decode(const uint8_t *data, int len)
{
    tsdu_d_call_start_t *tsdu = tsdu_create(tsdu_d_call_start_t, 0);
    if (!tsdu) {
        return NULL;
    }

    tsdu->has_key_reference = false;
    tsdu->has_key_of_call = false;
    if (len == 1) {
        return tsdu;
    }

    CHECK_LEN(len, 2, tsdu);
    ++data;
    --len;

    while (len > 0) {
        const int iei = data[0];
        ++data;
        --len;
        switch (iei) {
            case IEI_KEY_REFERENCE:
                tsdu->has_key_reference = true;
                tsdu->key_reference._data = data[0];
                ++data;
                --len;
                break;

            case IEI_KEY_OF_CALL:
                if (data[0] > sizeof(key_of_call_t)) {
                    LOG(WTF, "Wrong IEI size %d", data[0]);
                } else {
                    tsdu->has_key_of_call = true;
                    memcpy(tsdu->key_of_call, &data[1], data[0]);
                }
                len -= data[0] + 1;
                data += data[0] + 1;
                break;

            default:
                LOG(WTF, "Unexpected IEI 0x%x", iei);
                tsdu_destroy(&tsdu->base);
                return NULL;
        }
    }
    if (len < 0) {
        LOG(WTF, "Bufer underflow, len = %d", len);
    }

    return tsdu;
}

static tsdu_d_call_connect_t *d_call_connect_decode(const uint8_t *data, int len)
{
    tsdu_d_call_connect_t *tsdu = tsdu_create(tsdu_d_call_connect_t, 0);
    if (!tsdu) {
        return NULL;
    }
    CHECK_LEN(len, 15, tsdu);

    tsdu->call_type._data       = data[1];
    tsdu->channel_id            = get_bits(12, &data[2], 4);
    tsdu->u_ch_scrambling       = data[4];
    tsdu->d_ch_scrambling       = data[5];
    tsdu->key_reference._data   = data[6];
    memcpy(tsdu->valid_rt, &data[7], SIZEOF(tsdu_d_call_connect_t, valid_rt));
    tsdu->has_key_of_call =
        (tsdu->key_reference.key_type == KEY_TYPE_ESC) &&
        (tsdu->key_reference.key_index == KEY_INDEX_KEY_SUPPLIED);
    if (tsdu->has_key_of_call) {
        CHECK_LEN(len, 31, tsdu);
        memcpy(tsdu->key_of_call, &data[15], sizeof(key_of_call_t));
    }

    return tsdu;
}

static tsdu_u_registration_req_t *u_registration_req_decode(const uint8_t *data, int len)
{
    tsdu_u_registration_req_t *tsdu = tsdu_create(tsdu_u_registration_req_t, 0);
    if (!tsdu) {
        return NULL;
    }

    CHECK_LEN(len, 15, tsdu);

    const uint8_t *adr_data = &data[1];
    if (address_decode(&tsdu->host_adr, &adr_data)) {
        LOG(ERR, "Only single address ACK is supported");
    }
    tsdu->serial_nb[0]          = get_bits(4, data + 7, 0);
    tsdu->serial_nb[1]          = get_bits(4, data + 7, 4);
    tsdu->serial_nb[2]          = get_bits(4, data + 8, 0);
    tsdu->serial_nb[3]          = get_bits(4, data + 8, 4);
    tsdu->serial_nb[4]          = get_bits(4, data + 9, 0);
    tsdu->serial_nb[5]          = get_bits(4, data + 9, 4);
    tsdu->serial_nb[6]          = get_bits(4, data + 10, 0);
    tsdu->serial_nb[7]          = get_bits(4, data + 10, 4);
    tsdu->reg_seq               = get_bits(16, data + 11, 0);
    tsdu->complete_reg          = data[12];
    tsdu->rt_status._data       = data[13];

    return tsdu;
}

static tsdu_u_data_request_t *u_data_request_decode(const uint8_t *data, int len)
{
    CHECK_LEN(len, 5, NULL);

    tsdu_u_data_request_t *tsdu = tsdu_create(tsdu_u_data_request_t, 0);
    if (!tsdu) {
        return NULL;
    }

    tsdu->trans_mode =      get_bits(4, &data[1], 4);
    tsdu->trans_param1 =    get_bits(16, &data[2], 0);
    tsdu->trans_param2 =    get_bits(16, &data[4], 0);

    return tsdu;
}

static tsdu_u_authentication_t *
u_authentication_decode(const uint8_t *data, int len)
{
    tsdu_u_authentication_t *tsdu = tsdu_create(tsdu_u_authentication_t, 0);
    if (!tsdu) {
        return NULL;
    }
    CHECK_LEN(len, 6, tsdu);

    tsdu->val             = data[1];
    memcpy(tsdu->result_rt, &data[2], sizeof(tsdu->result_rt));

    return tsdu;
}

static tsdu_u_terminate_t *
u_terminate_decode(const uint8_t *data, int len)
{
    tsdu_u_terminate_t *tsdu = tsdu_create(tsdu_u_terminate_t, 0);
    if (!tsdu) {
        return NULL;
    }
    CHECK_LEN(len, 1, tsdu);

    tsdu->cause           = data[1];

    return tsdu;
}

static tsdu_u_call_connect_t *
u_call_connect_decode(const uint8_t *data, int len)
{
    tsdu_u_call_connect_t *tsdu = tsdu_create(tsdu_u_call_connect_t, 0);
    if (!tsdu) {
        return NULL;
    }
    CHECK_LEN(len, 6, tsdu);

    tsdu->val             = data[1];
    memcpy(tsdu->result_rt, &data[2], sizeof(tsdu->result_rt));

    return tsdu;
}

int tsdu_decode(const uint8_t *data, int len, tsdu_t **tsdu)
{
    if (len < 1) {
        LOG(ERR, "%d data too short %d < %d", __LINE__, len, 1);
        return -1;
    }
    if (!tsdu) {
        LOG(ERR, "tsdu == NULL");
        return -1;
    }

    const codop_t codop = get_bits(8, data, 0);

    *tsdu = NULL;
    switch (codop) {
        case D_ABILITY_MNGT:
            *tsdu = (tsdu_t *)d_ability_mngt_decode(data, len);
            break;

        case D_ADDITIONAL_PARTICIPANTS:
            *tsdu = (tsdu_t *)d_additional_participants_decode(data, len);
            break;

        case D_AUTHENTICATION:
            *tsdu = (tsdu_t *)d_authentication_decode(data, len);
            break;

        case D_AUTHORISATION:
            *tsdu = (tsdu_t *)d_authorisation_decode(data, len);
            break;

        case D_CALL_ALERT:
            *tsdu = (tsdu_t *)d_call_alert_decode(data, len);
            break;

        case D_CALL_CONNECT:
            *tsdu = (tsdu_t *)d_call_connect_decode(data, len);
            break;

        case D_CALL_START:
            *tsdu = (tsdu_t *)d_call_start_decode(data, len);
            break;

        case D_CALL_SETUP:
            *tsdu = (tsdu_t *)d_call_setup_decode(data, len);
            break;

        case D_CCH_OPEN:
            *tsdu = (tsdu_t *)d_cch_open_decode(data, len);
            break;

        case D_CONNECT_CCH:
            *tsdu = (tsdu_t *)d_connect_cch_decode(data, len);
            break;

        case D_CRISIS_NOTIFICATION:
            *tsdu = (tsdu_t *)d_crisis_notification_decode(data, len);
            break;

        case D_DATA_AUTHENTICATION:
            *tsdu = (tsdu_t *)d_data_authentication_decode(data, len);
            break;

        case D_DATA_END:
            *tsdu = (tsdu_t *)d_data_end_decode(data, len);
            break;

        case D_DATA_MSG_DOWN:
            *tsdu = (tsdu_t *)d_data_msg_down_decode(data, len);
            break;

        case D_DATA_REQUEST:
            *tsdu = (tsdu_t *)d_data_request_decode(data, len);
            break;

        case D_DATAGRAM:
            *tsdu = (tsdu_t *)d_datagram_decode(data, len);
            break;

        case D_DATAGRAM_NOTIFY:
            *tsdu = (tsdu_t *)d_datagram_notify_decode(data, len);
            break;

        case D_DCH_OPEN:
            *tsdu = (tsdu_t *)d_dch_open_decode(data, len);
            break;

        case D_ECH_OVERLOAD_ID:
            *tsdu = (tsdu_t *)d_ech_overload_id_decode(data, len);
            break;

        case D_EXPLICIT_SHORT_DATA:
            *tsdu = (tsdu_t *)d_explicit_short_data_decode(data, len);
            break;

        case D_FORCED_REGISTRATION:
            *tsdu = (tsdu_t *)d_forced_registration_decode(data, len);
            break;

        case D_GROUP_ACTIVATION:
            *tsdu = (tsdu_t *)d_group_activation_decode(data, len);
            break;

        case D_GROUP_COMPOSITION:
            *tsdu = (tsdu_t *)d_group_composition_decode(data, len);
            break;

        case D_GROUP_LIST:
            *tsdu = (tsdu_t *)d_group_list_decode(data, len);
            break;

        case D_GROUP_PAGING:
            *tsdu = (tsdu_t *)d_group_paging_decode(data, len);
            break;

        case D_GROUP_REJECT:
            *tsdu = (tsdu_t *)d_group_reject_decode(data, len);
            break;

        case D_HOOK_ON_INVITATION:
            *tsdu = (tsdu_t *)d_hook_on_invitation_decode(data, len);
            break;

        case D_LOCATION_ACTIVITY_ACK:
            *tsdu = (tsdu_t *)d_location_activity_ack_decode(data, len);
            break;

        case D_NEIGHBOURING_CELL:
            *tsdu = (tsdu_t *)d_neighbouring_cell_decode(data, len);
            break;

        case D_SYSTEM_INFO:
            *tsdu = (tsdu_t *)d_system_info_decode(data, len);
            break;

        case D_REGISTRATION_NAK:
            *tsdu = (tsdu_t *)d_registration_nak_decode(data, len);
            break;

        case D_REGISTRATION_ACK:
            *tsdu = (tsdu_t *)d_registration_ack_decode(data, len);
            break;

        case D_CONNECT_DCH:
            *tsdu = (tsdu_t *)d_connect_dch_decode(data, len);
            break;

        case D_REFUSAL:
            *tsdu = (tsdu_t *)d_refusal_decode(data, len);
            break;

        case D_REJECT:
            *tsdu = (tsdu_t *)d_reject_decode(data, len);
            break;

        case D_RELEASE:
            *tsdu = (tsdu_t *)d_release_decode(data, len);
            break;

        case D_RETURN:
            *tsdu = (tsdu_t *)d_return_decode(data, len);
            break;

        case D_GROUP_IDLE:
            *tsdu = (tsdu_t *)d_group_idle_decode(data, len);
            break;

        case U_AUTHENTICATION:
            *tsdu = (tsdu_t *)u_authentication_decode(data, len);
            break;

        case U_CALL_CONNECT:
            *tsdu = (tsdu_t *)u_call_connect_decode(data, len);
            break;

        case U_DATA_REQUEST:
            *tsdu = (tsdu_t *)u_data_request_decode(data, len);
            break;

        case U_REGISTRATION_REQ:
            *tsdu = (tsdu_t *)u_registration_req_decode(data, len);
            break;

        case U_TERMINATE:
            *tsdu = (tsdu_t *)u_terminate_decode(data, len);
            break;

        case D_ACCESS_DISABLED:
        case D_BACK_CCH:
        case D_BROADCAST:
        case D_BROADCAST_NOTIFICATION:
        case D_BROADCAST_WAITING:
        case D_CALL_ACTIVATION:
        case D_CALL_COMPOSITION:
        case D_CALL_END:
        case D_CALL_OVERLOAD_ID:
        case D_CALL_SWITCH:
        case D_CALL_WAITING:
        case D_DATA_DOWN_STATUS:
        case D_DATA_SERV:
        case D_DEVIATION_ON:
        case D_ECCH_DESCRIPTION:
        case D_ECH_ACTIVATION:
        case D_ECH_REJECT:
        case D_EMERGENCY_ACK:
        case D_EMERGENCY_NAK:
        case D_EMERGENCY_NOTIFICATION:
        case D_EXTENDED_STATUS:
        case D_FUNCTIONAL_SHORT_DATA:
        case D_GROUP_END:
        case D_GROUP_OVERLOAD_ID:
        case D_CHANNEL_INIT:
        case D_INFORMATION_DELIVERY:
        case D_OC_ACTIVATION:
        case D_OC_PAGING:
        case D_OC_REJECT:
        case D_PRIORITY_GRP_ACTIVATION:
        case D_PRIORITY_GRP_WAITING:
        case D_SERVICE_DISABLED:
        case D_TRAFFIC_DISABLED:
        case D_TRAFFIC_ENABLED:
        case D_TRANSFER_NAK:
        case U_ABORT:
        case U_CALL_ANSWER:
        case U_CALL_INTRUSION_ECH:
        case U_CALL_INTRUSION_OCH:
        case U_CALL_INTRUSION_PC:
        case U_CALL_RELEASE:
        case U_CALL_SETUP:
        case U_DATA_DOWN_ACCEPT:
        case U_DATA_MSG_UP:
        case U_DEVIATION_CLEAR:
        case U_DEVIATION_SET:
        case U_ECH_CLOSE:
        case U_ECH_SETUP:
        case U_EMERGENCY_REQ:
        case U_END:
        case U_ERROR_REPORT:
        case U_EVENT_REPORT:
        case U_LOCATION_ACTIVITY:
        case U_OCH_RELEASE:
        case U_OCH_SETUP:
        case U_TRANSFER_REQ:
            LOG(ERR, "Unsupported codop 0x%02x", codop);
            *tsdu = (tsdu_t *)d_unknown_parse(data, len);
            break;

        default:
            LOG(WTF, "Unknown codop=0x%02x", codop);
            *tsdu = (tsdu_t *)d_unknown_parse(data, len);
            break;
    }

    if (*tsdu) {
        (*tsdu)->codop = codop;
    }

    return 0;
}

