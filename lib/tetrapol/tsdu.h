#pragma once

#include <tetrapol/addr.h>
#include <tetrapol/msg_coding.h>

#include <stdbool.h>
#include <stdint.h>

/**
  PAS 0001-3-2 4.4
  X_UNKNOWN_xx messages float in air but are not part of public specification
  */
enum {
    D_REJECT                    = 0x08,
    D_REFUSAL                   = 0x09,
    U_END                       = 0x0a,
    D_BACK_CCH                  = 0x0b,
    D_RELEASE                   = 0x0c,
    U_ABORT                     = 0x0d,
    U_TERMINATE                 = 0x0e,
    D_HOOK_ON_INVITATION        = 0x0f,
    D_RETURN                    = 0x10,
    U_EVENT_REPORT              = 0x11,
    D_CALL_WAITING              = 0x12,
    D_AUTHENTICATION            = 0x13,
    U_AUTHENTICATION            = 0x14,
    D_AUTHORISATION             = 0x16,
    U_ERROR_REPORT              = 0x17,
    D_CHANNEL_INIT              = 0x18,
    U_REGISTRATION_REQ          = 0x20,
    D_REGISTRATION_NAK          = 0x21,
    D_REGISTRATION_ACK          = 0x22,
    D_FORCED_REGISTRATION       = 0x23,
    U_LOCATION_ACTIVITY         = 0x24,
    D_LOCATION_ACTIVITY_ACK     = 0x25,
    U_CALL_SETUP                = 0x30,
    D_CALL_ALERT                = 0x31,
    D_CALL_SETUP                = 0x32,
    U_CALL_ANSWER               = 0x33,
    D_CALL_CONNECT              = 0x34,
    D_CALL_SWITCH               = 0x35,
    U_CALL_INTRUSION_PC         = 0x36,
    U_CALL_INTRUSION_OCH        = 0x37,
    D_TRANSFER_NAK              = 0x39,
    U_TRANSFER_REQ              = 0x3a,
    U_CALL_INTRUSION_ECH        = 0x3b,
    U_CALL_RELEASE              = 0x3c,
    U_CALL_CONNECT              = 0x3d,
    U_CALL_SWITCH               = 0x3d,
    D_CALL_START                = 0x3e,
    D_CALL_ACTIVATION           = 0xe0,
    D_CALL_COMPOSITION          = 0xe1,
    D_CALL_END                  = 0xe2,
    D_CALL_OVERLOAD_ID          = 0xe3,
    D_FUNCTIONAL_SHORT_DATA     = 0x42,
    U_DATA_DOWN_ACCEPT          = 0x43,
    U_DATA_MSG_UP               = 0x44,
    D_DATA_MSG_DOWN             = 0x45,
    D_EXPLICIT_SHORT_DATA       = 0x46,
    D_DATA_END                  = 0x48,
    D_DATAGRAM_NOTIFY           = 0x49,
    D_DATAGRAM                  = 0x4a,
    D_BROADCAST                 = 0x4b,
    D_DATA_SERV                 = 0x4c,
    D_DATA_DOWN_STATUS          = 0x4e,
    D_CONNECT_DCH               = 0x60,
    D_CONNECT_CCH               = 0x62,
    D_DATA_AUTHENTICATION       = 0x63,
    D_DATA_REQUEST              = 0x64,
    D_DCH_OPEN                  = 0x65,
    U_DATA_REQUEST              = 0x66,
    D_EXTENDED_STATUS           = 0x67,
    D_CCH_OPEN                  = 0x68,
    D_BROADCAST_WAITING         = 0x69,
    U_OCH_RELEASE               = 0x50,
    U_OCH_SETUP                 = 0x51,
    U_ECH_CLOSE                 = 0x52,
    D_EMERGENCY_NOTIFICATION    = 0x53,
    U_ECH_SETUP                 = 0x54,
    D_GROUP_ACTIVATION          = 0x55,
    D_ECH_ACTIVATION            = 0x56,
    D_GROUP_END                 = 0x57,
    D_GROUP_IDLE                = 0x58,
    D_GROUP_REJECT              = 0x59,
    D_ECH_REJECT                = 0x5a,
    D_GROUP_PAGING              = 0x5b,
    D_BROADCAST_NOTIFICATION    = 0x5c,
    D_CRISIS_NOTIFICATION       = 0x5d,
    D_EMERGENCY_ACK             = 0x5f,
    D_EMERGENCY_NAK             = 0x80,
    U_EMERGENCY_REQ             = 0x81,
    D_GROUP_OVERLOAD_ID         = 0x82,
    D_ECH_OVERLOAD_ID           = 0x83,
    D_PRIORITY_GRP_WAITING      = 0x84,
    D_PRIORITY_GRP_ACTIVATION   = 0x85,
    D_OC_ACTIVATION             = 0x86,
    D_OC_REJECT                 = 0x87,
    D_OC_PAGING                 = 0x88,
    D_ACCESS_DISABLED           = 0x70,
    D_TRAFFIC_ENABLED           = 0x71,
    D_TRAFFIC_DISABLED          = 0x72,
    U_DEVIATION_CLEAR           = 0x73,
    U_DEVIATION_SET             = 0x74,
    D_DEVIATION_ON              = 0x76,
    D_SERVICE_DISABLED          = 0x78,
    D_ABILITY_MNGT              = 0x77,
    D_SYSTEM_INFO               = 0x90,
    D_GROUP_LIST                = 0x92,
    D_GROUP_COMPOSITION         = 0x93,
    D_NEIGHBOURING_CELL         = 0x94,
    D_ECCH_DESCRIPTION          = 0x95,
    D_ADDITIONAL_PARTICIPANTS   = 0x96,
    D_INFORMATION_DELIVERY      = 0xc5,
};
typedef uint8_t codop_t;

extern const int CELL_RADIO_PARAM_PWR_TX_ADJUST_TO_DBM[16];
extern const int CELL_RADIO_PARAM_RX_LEV_ACCESS_TO_DBM[16];

enum {
    IEI_GROUP_ID            = 0x01,
    IEI_CELL_ID_LIST        = 0x02,
    IEI_KEY_OF_CALL         = 0x03,
    IEI_ADJACENT_BN_LIST    = 0x04,
    IEI_TTI                 = 0x05,
    IEI_ADR                 = 0x06,
    IEI_USER_PRIORITY       = 0x81,
    IEI_COVERAGE_ID         = 0x82,
    IEI_KEY_REFERENCE       = 0x83,
    IEI_ADD_SETUP_PARAM     = 0x84,
    IEI_PROFILE_ID          = 0x85,
};

// do not use directly, this struct must be first member of each TSDU structure
typedef struct {
    codop_t codop;
    int noptionals;     ///< number of optionals
    /**
      In subclassed TSDU structure, noptionals pointers should be present.
      Those are initialized to NULL by tsdu_base_init and freed by tsdu_destroy.
      */
    void *optionals[];
} tsdu_base_t;

/// structure for commands not in specification
typedef struct {
    tsdu_base_t base;
    uint8_t *data;
    int len;
} tsdu_unknown_codop_t;

/// PAS 0001-3-2 4.4.1
typedef struct {
    tsdu_base_t base;
    uint16_t data_len;
    uint8_t data[2048];
} tsdu_d_ability_mngt_t;

/// PAS 0001-3-2 4.4.4
typedef struct {
    tsdu_base_t base;
    address_list_t *calling_adr;
    uint8_t coverage_id;
} tsdu_d_additional_participants_t;

/// PAS 0001-3-2 4.4.5
typedef struct {
    tsdu_base_t base;
    key_reference_t key_reference;
    uint8_t valid_rt[8];
} tsdu_d_authentication_t;

/// PAS 0001-3-2 4.5.6
typedef struct {
    tsdu_base_t base;
    bool has_key_reference;
    key_reference_t key_reference;
} tsdu_d_authorisation_t;

/// PAS 0001-3-2 4.4.11
typedef struct {
    tsdu_base_t base;
} tsdu_d_call_alert_t;

/// PAS 0001-3-2 4.4.12
typedef struct {
    tsdu_base_t base;
    call_type_t call_type;
    uint16_t channel_id;
    uint8_t u_ch_scrambling;
    uint8_t d_ch_scrambling;
    key_reference_t key_reference;
    uint8_t valid_rt[8];
    bool has_key_of_call;
    key_of_call_t key_of_call;
} tsdu_d_call_connect_t;

/// PAS 0001-3-2 4.4.13
typedef struct {
    tsdu_base_t base;
    address_t calling_adr;
    bool has_add_setup_param;
    add_setup_param_t add_setup_param;
} tsdu_d_call_setup_t;

/// PAS 0001-3-2 4.4.14
typedef struct {
    tsdu_base_t base;
    bool has_key_reference;
    key_reference_t key_reference;
    bool has_key_of_call;
    key_of_call_t key_of_call;
} tsdu_d_call_start_t;

/// 0001-3-2 4.4.17
typedef struct {
    tsdu_base_t base;
} tsdu_d_cch_open_t;

/// 0001-3-2 4.4.19
typedef struct {
    tsdu_base_t base;
} tsdu_d_connect_cch_t;

/// PAS 0001-3-2 4.4.20
typedef struct {
    tsdu_base_t base;
    uint8_t dch_low_layer;
    uint16_t channel_id;
    uint8_t u_ch_scrambling;
    uint8_t d_ch_scrambling;
} tsdu_d_connect_dch_t;

/// PAS 0001-3-2 4.4.21
typedef struct {
    tsdu_base_t base;
    address_t calling_adr;
    uint8_t organisation;
    uint8_t coverage_id;
    uint8_t og_nb;
    uint16_t group_ids[5];
} tsdu_d_crisis_notification_t;

/// PAS 0001-3-2 4.4.22
typedef struct {
    tsdu_base_t base;
    key_reference_t key_reference_auth;
    uint8_t valid_rt[8];
    key_reference_t key_reference_ciph;
} tsdu_d_data_authentication_t;

/// PAS 0001-3-2 4.4.24
typedef struct {
    tsdu_base_t base;
    uint8_t cause;
} tsdu_d_data_end_t;

/// PAS 0001-3-2 5.3.25
/// PAS 0001-13-2 data, should be implemented in application layer
typedef struct {
    tsdu_base_t base;
    uint16_t data_len;
    uint8_t data[2048];
} tsdu_d_data_msg_down_t;

/// PAS 0001-3-2 4.4.26
typedef struct {
    tsdu_base_t base;
    key_reference_t key_reference_auth;
    uint8_t valid_rt[8];
    key_reference_t key_reference_ciph;
    trans_mode_t trans_mode;
    uint16_t trans_param1;
    uint16_t trans_param2;
    bool has_trans_param3;
    uint16_t trans_param3;
} tsdu_d_data_request_t;

/// PAS 0001-3-2 4.4.27
typedef struct {
    tsdu_base_t base;
    uint8_t call_priority;
    uint16_t message_reference;
    key_reference_t key_reference;
    int len;
    uint8_t data[];
} tsdu_d_datagram_t;

/// PAS 0001-3-2 4.4.28
typedef struct {
    tsdu_base_t base;
    uint8_t call_priority;
    uint16_t message_reference;
    key_reference_t key_reference;
    int destination_port;
} tsdu_d_datagram_notify_t;

/// PAS 0001-3-2 4.4.29
typedef struct {
    tsdu_base_t base;
} tsdu_d_dch_open_t;

/// PAS 0001-3-2 4.4.33
typedef struct {
    tsdu_base_t base;
    activation_mode_t activation_mode;
    uint16_t group_id;
    cell_id_t cell_id;
    uint8_t organisation;
} tsdu_d_ech_overload_id_t;

/// PAS 0001-3-2 4.4.39
typedef struct {
    tsdu_base_t base;
    int len;
    uint8_t data[];
} tsdu_d_explicit_short_data_t;

/// PAS 0001-3-2 4.4.41
typedef struct {
    tsdu_base_t base;
    address_t calling_adr;
} tsdu_d_forced_registration_t;

/// PAS 0001-3-2 4.4.43
typedef struct {
    tsdu_base_t base;
    // pointers must be right after base
    activation_mode_t activation_mode;
    uint16_t group_id;
    uint8_t coverage_id;
    uint16_t channel_id;
    uint8_t u_ch_scrambling;
    uint8_t d_ch_scrambling;
    key_reference_t key_reference;
    bool has_addr_tti;
    addr_t addr_tti;
} tsdu_d_group_activation_t;

/// PAS 0001-3-2 4.4.44
typedef struct {
    tsdu_base_t base;
    uint16_t group_id;
    uint8_t og_nb;
    uint16_t group_ids[16];   ///< limit shoudl be 10, but og_ng have 4b
} tsdu_d_group_composition_t;

/// PAS 0001-3-2 4.4.46
typedef struct {
    tsdu_base_t base;
    uint8_t cause;
} tsdu_d_group_idle_t;

/// PAS 0001-3-2 4.4.47
typedef struct {
    uint8_t coverage_id;
    uint8_t call_priority;
    uint16_t group_id;
    och_parameters_t och_parameters;
    uint16_t neighbouring_cell;
} tsdu_d_group_list_open_t;

typedef struct {
    uint8_t coverage_id;
    uint16_t neighbouring_cell;
} tsdu_d_group_list_talk_group_t;

typedef struct {
    cell_id_t cell_id;
} tsdu_d_group_list_emergency_t;

typedef struct {
    tsdu_base_t base;
    // see variable-lenght array in base
    tsdu_d_group_list_emergency_t *emergency;
    tsdu_d_group_list_talk_group_t *group;
    tsdu_d_group_list_open_t *open;
    uint8_t nemergency;
    uint8_t ngroup;
    uint8_t nopen;

    reference_list_t reference_list;
    index_list_t index_list;
} tsdu_d_group_list_t;

/// PAS 0001-3-2 4.4.49
typedef struct {
    tsdu_base_t base;
    activation_mode_t activation_mode;
    uint16_t group_id;
    uint8_t coverage_id;
    key_reference_t key_reference;
} tsdu_d_group_paging_t;

/// PAS 0001-3-2 4.4.50
typedef struct {
    tsdu_base_t base;
    activation_mode_t activation_mode;
    uint16_t group_id;
    uint8_t coverage_id;
    uint8_t _zero;
    uint8_t cause;
} tsdu_d_group_reject_t;

/// PAS 0001-3-2 4.4.51
typedef struct {
    tsdu_base_t base;
    uint8_t cause;
} tsdu_d_hook_on_invitation_t;

/// PAS 0001-3-2 4.4.56
typedef struct {
    tsdu_base_t base;
    rt_status_t rt_status;
} tsdu_d_location_activity_ack_t;

/// PAS 0001-3-2 4.4.57
typedef struct {
    tsdu_base_t base;
    cell_id_list_t *cell_ids;   // iei=cell_id_list;
    cell_bn_list_t *cell_bns;   // iei=adjecent_bn_list;
    ccr_config_t ccr_config;
    uint8_t ccr_param;

    struct {
        uint8_t bn_nb;
        uint16_t channel_id;
        adjacent_param_t adjacent_param;
    } adj_cells[16];
} tsdu_d_neighbouring_cell_t;

/// PAS 0001-3-2 4.4.63
typedef struct {
    tsdu_base_t base;
    uint8_t cause;
} tsdu_d_refusal_t;

/// PAS 0001-3-2 4.4.64
typedef struct {
    tsdu_base_t base;
    uint8_t complete_reg;
    uint8_t rt_min_activity;
    rt_status_t rt_status;
    address_t host_adr;
    uint8_t rt_min_registration;
    uint8_t tlr_value;
    rt_data_info_t rt_data_info;
    uint16_t group_id;
    bool has_coverage_id;
    uint8_t coverage_id;
} tsdu_d_registration_ack_t;

/// PAS 0001-3-2 4.4.65
typedef struct {
    tsdu_base_t base;
    uint8_t cause;
    address_t host_adr;
    uint8_t bn_id;
    cell_id_t cell_id;
} tsdu_d_registration_nak_t;

/// PAS 0001-3-2 4.4.66
typedef struct {
    tsdu_base_t base;
    uint8_t cause;
} tsdu_d_reject_t;

/// PAS 0001-3-2 4.4.67
typedef struct {
    tsdu_base_t base;
    uint8_t cause;
} tsdu_d_release_t;

/// PAS 0001-3-2 4.4.68
typedef struct {
    tsdu_base_t base;
    uint8_t cause;
} tsdu_d_return_t;

/// PAS 0001-3-2 4.4.82
typedef struct {
    tsdu_base_t base;
    uint8_t val;
    uint8_t result_rt[4];
} tsdu_u_authentication_t;

/// PAS 0001-3-2 4.4.86
typedef struct {
    tsdu_base_t base;
    uint8_t val;
    uint8_t result_rt[4];
} tsdu_u_call_connect_t;

/// PAS 0001-3-2 4.4.71
typedef struct {
    tsdu_base_t base;
    cell_state_t cell_state;
    cell_config_t cell_config;
    uint8_t country_code;
    system_id_t system_id;
    loc_area_id_t loc_area_id;
    uint8_t bn_id;
    cell_id_t cell_id;
    cell_bn_t cell_bn;
    uint8_t u_ch_scrambling;
    cell_radio_param_t cell_radio_param;
    uint8_t system_time;
    cell_access_t cell_access;
    uint8_t _unused_1;  // 4b are marked as unused
    uint16_t superframe_cpt;
// used only when cell_state == disconnected
    uint8_t band;
    uint16_t channel_id;
} tsdu_d_system_info_t;

/// PAS 0001-3-2 4.4.97
typedef struct {
    tsdu_base_t base;
    trans_mode_t trans_mode;
    uint16_t trans_param1;
    uint16_t trans_param2;
} tsdu_u_data_request_t;

/// PAS 0001-3-2 4.4.117
typedef struct {
    tsdu_base_t base;
    address_t host_adr;
    uint8_t system_id;
    uint8_t serial_nb[8];
    uint8_t counter_bn;
    uint8_t counter_rsw;
    uint16_t reg_seq;
    uint8_t complete_reg;
    rt_status_t rt_status;
} tsdu_u_registration_req_t;

/// PAS 0001-3-2 4.4.121
typedef struct {
    tsdu_base_t base;
    uint8_t cause;
} tsdu_u_terminate_t;

// this might change in future
typedef tsdu_base_t tsdu_t;

void tsdu_destroy(tsdu_base_t *tsdu);

/**
 * @brief tsdu_d_decode Compose TSDU from TPDUs.
 * @param data TPDU composed data.
 * @param len length of data in bytes
 * @param tsdu Is set to point o decoded TSDU when available or to NULL
 *   otherwise. Caller is responsible for freeing this TSDU.
 * @return 0 on success, -1 on fail.
 */
int tsdu_decode(const uint8_t *data, int len, tsdu_t **tsdu);

