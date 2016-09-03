#pragma once

#include <stdbool.h>
#include <stdint.h>


/// PAS 0001-3-2 5.3.2
enum {
    ACTIVATION_MODE_HOOK_OPEN = 0,
    ACTIVATION_MODE_HOOK_INTERNAL = 1,
    ACTIVATION_MODE_HOOK_BROADCAST = 2,
    ACTIVATION_MODE_HOOK_EXTERNAL = 3,
};

enum {
    ACTIVATION_MODE_TYPE_WITHOUT_TONE = 0,
    ACTIVATION_MODE_TYPE_WITH_TONE = 1,
    ACTIVATION_MODE_TYPE_RING = 2,
    ACTIVATION_MODE_TYPE_RESERVER = 3,
};

typedef struct {
    uint8_t hook;
    uint8_t type;
} activation_mode_t;

/// PAS 0001-3-2 5.3.3
typedef union {
    struct {
        unsigned int origin : 3;
        unsigned int _reserver : 2;
        unsigned int id : 1;
        unsigned int mod : 1;
        unsigned int sil : 1;
    };
    uint8_t _data;
} add_setup_param_t;

/// PAS 0001-3-2 5.3.4
enum {
    ADDRESS_CNA_NOT_SIGNIFICANT = 0,
    ADDRESS_CNA_RFSI = 1,
    ADDRESS_CNA_X400 = 2,
    ADDRESS_CNA_PABX = 3,
    ADDRESS_CNA_FUNCTIONAL = 4,
    ADDRESS_CNA_LONG = 5,
    ADDRESS_CNA_BINARY = 6,
    ADDRESS_CNA_ESCAPED_CODE = 7,
};

typedef union {
    struct {
        uint8_t r[3];
        uint8_t f;
        uint8_t s[2];
        uint8_t i[3];
    };
    uint8_t addr[9];
} rfsi_address_t;

typedef struct {
    uint8_t cna;
    int len;    //< real lenght of address
    union {
        rfsi_address_t rfsi;
        uint8_t pabx[15];
        // TODO: add another address types
    };
} address_t;

typedef struct {
    int nadrs;
    address_t called_adr[];
} address_list_t;

/// PAS 0001-3-2 5.3.5
enum {
    ADJACENT_PARAM_BN_DIFFERENT = 0,
    ADJACENT_PARAM_BN_SAME = 1,
};

enum {
    ADJACENT_PARAM_LOC_DIFFERENT = 0,
    ADJACENT_PARAM_LOC_SAME = 1,
};

enum {
    ADJACENT_PARAM_EXP_NORMAL = 0,
    ADJACENT_PARAM_EXP_EXPERIMENTAL = 1,
};

typedef union {
    uint8_t _data;
    struct {
        unsigned int rxlev_access : 4;
        unsigned int exp : 1;
        unsigned int _reserved : 1;
        unsigned int loc : 1;
        unsigned int bn : 1;
    };
} adjacent_param_t;

/// PAS 0001-3-2 5.3.9
enum {
    BN_ID_UNDEFINED = 0,
};

/// PAS 0001-3-2 5.3.14
enum {
    CALL_PRIORITY_NOT_SIGNIFICANT = 0,
    CALL_PRIORITY_ROUTINE = 2,
    CALL_PRIORITY_URGENT = 4,
    CALL_PRIORITY_FLASH = 6,
    CALL_PRIORITY_BROADCAST = 8,
    CALL_PRIORITY_CRISIS = 10,
    CALL_PRIORITY_EMERGENCY = 12,
    CALL_PRIORITY_TOWER_COMMUNICATION = 13,
};

/// PAS 0001-3-2 5.3.15
enum {
    ORIGIN_TETRAPOL = 0,
    ORIGIN_PABX = 1,
    ORIGIN_DISPATCH_CENTRE = 2,
};

enum {
    DESTINATION_TETRAPOL = 0,
    DESTINATION_PABX = 1,
    DESTINATION_DISPATCH_CENTRE = 2,
    DESTINATION_MULTIPLE = 7,
};

typedef union {
    uint8_t _data;
    struct {
        unsigned int origin : 3;
        unsigned int destination : 3;
        unsigned int trfs : 1;
        unsigned int _zero : 1;
    };
} call_type_t;

/// PAS 0001-3-2 5.3.16
typedef struct {
    uint8_t number;
} ccr_config_t;

/// PAS 0001-3-2 5.3.18
typedef union {
    uint8_t _data;
    struct {
        unsigned int min_service_class : 4;
        unsigned int min_reg_class : 4;
    };
} cell_access_t;

/// PAS 0001-3-2 5.3.19
typedef union {
    uint16_t _data;
    struct {
        uint8_t r3 : 4;
        uint8_t r2 : 4;
        uint8_t r1 : 4;
    };
} cell_bn_t;

typedef struct {
    uint8_t len;
    cell_bn_t cell_bn[];
} cell_bn_list_t;

/// PAS 0001-3-2 5.3.20
enum {
    CELL_CONFIG_DC_SINGLE           = 0,
    CELL_CONFIG_DC_DOUBLE_COVERAGE  = 1,
};

enum {
    CELL_CONFIG_SIM_SINGLE      = 0,
    CELL_CONFIG_SIM_SIMULCAST   = 1,
};

/// PAS 0001-3-2 5.3.21
enum {
    CELL_ID_FORMAT_0    = 0,
    CELL_ID_FORMAT_1    = 1,
    // 2 values reserved
};

typedef struct {
    uint8_t bs_id;
    uint8_t rsw_id;
} cell_id_t;

typedef struct {
    int len;
    cell_id_t cell_ids[];
} cell_id_list_t;

/// PAS 0001-3-2 5.3.22
enum {
    CELL_RADIO_PARAM_TX_MAX_UNLIMITED   = 0,
    // 7 values reserved
};

enum {
    CELL_RADIO_PARAM_RADIO_LINK_TIMEOUT_INTERNAL = 0,
    // 31 values reserved
};

/// shall by used by RT to estimate radio conditions for one cell
extern const int CELL_RADIO_PARAM_RX_LEV_ACCESS_TO_DBM[16];

/// convert pwr_tx_adjust to TX power adjustment in dBm
extern const int CELL_RADIO_PARAM_PWR_TX_ADJUST_TO_DBM[16];

typedef struct {
    uint8_t tx_max;
    uint8_t radio_link_timeout;
    uint8_t pwr_tx_adjust;
    uint8_t rx_lev_access;
} cell_radio_param_t;

/// PAS 0001-3-2 5.3.23
enum {
    CELL_STATE_EXP_NORMAL = 0,
    CELL_STATE_EXP_EXPERIMENTAL = 1,
};

enum {
    CELL_STATE_ROAM_ACCEPTED    = 0,
    CELL_STATE_ROAM_HOME_ONLY   = 1,
};

enum {
    CELL_STATE_MODE_NORMAL              = 0,
    CELL_STATE_MODE_DISC_INTERN_BN      = 1,
    CELL_STATE_MODE_DISC_MAIN_SWITCH    = 2,
    CELL_STATE_MODE_DISC_RADIOSWITCH    = 3,
    CELL_STATE_MODE_DISC_BSC            = 4,
    // 3 values reserved
};

typedef union {
    uint8_t _data;
    struct {
        unsigned int exp : 1;
        unsigned int roam : 1;
        unsigned int _reserved_00 : 2;
        unsigned int bch : 1;
        unsigned int mode : 3;
    };
} cell_state_t;

enum {
    CELL_CONFIG_MUX_TYPE_DEFAULT    = 0,
    CELL_CONFIG_MUX_TYPE_TYPE_2     = 1,
    // 6 values reserved
};

enum {
    CELL_CONFIG_ATTA_NOT_SUPPORTED  = 0,
    CELL_CONFIG_ATTA_SUPPORTED      = 1,
};

enum {
    CELL_CONFIG_ECCH_NOT_PRESENT    = 0,
    CELL_CONFIG_ECCH_PRESENT        = 1,
};

typedef union {
    uint8_t _data;
    struct {
        unsigned int dc : 1;
        unsigned int sim : 1;
        unsigned int mux_type : 3;
        unsigned int _reserved_0 : 1;
        unsigned int atta : 1;
        unsigned int eccch : 1;
    };
} cell_config_t;

/// PAS 0001-3-2 5.3.27
enum {
    COVERAGE_ID_NA = 0,
    COVERAGE_ID_RESERVER = 1,
    // 254 unused values
};

/// PAS 0001-3-2 5.3.36

enum {
    INDEX_LIST_MODE_NOT_SIGNIFICANT = 0,
    INDEX_LIST_MODE_PCH_ONLY = 1,
    INDEX_LIST_MODE_ACT_BITMAP_ONLY = 2,
    INDEX_LIST_MODE_PCH_BITMAP = 3,
};

typedef union {
    uint8_t _data;
    struct {
        unsigned int index : 6;
        unsigned int mode : 2;
    };
} index_list_t;

/// PAS 0001-3-2 5.3.37
typedef uint8_t key_i_t;

/// PAS 0001-3-2 5.3.38
typedef uint8_t key_of_call_t[16];

/// PAS 0001-3-2 5.3.39
enum {
    KEY_TYPE_RNK = 0,
    KEY_TYPE_RNK_PLUS = 1,
    KEY_TYPE_FRNK = 2,
    KEY_TYPE_NNK = 3,
    KEY_TYPE_FNNK = 4,
    KEY_TYPE_FAK = 5,
    // 9 values reserved
    KEY_TYPE_ESC = 15,
};

enum {
    KEY_INDEX_CLEAR_CALL = 0,
    KEY_INDEX_TKK = 14,
    KEY_INDEX_KEY_SUPPLIED = 15,
    // 15, ciphering key is in TSDU/mode shall be chosen by SwMI
};

typedef union {
    uint8_t _data;
    struct {
        unsigned int key_index : 4;
        unsigned int key_type : 4;
    };
} key_reference_t;

/// PAS 0001-3-2 5.3.40
enum {
    LOCAL_AREA_ID_MODE_RSW_BS   = 0,
    LOCAL_AREA_ID_MODE_BS       = 1,
    LOCAL_AREA_ID_MODE_LOC      = 2,
    // one value reserved
};

typedef union {
    uint8_t _data;
    struct {
        unsigned int loc_id : 6;
        unsigned int mode : 2;
    };
} loc_area_id_t;

/// PAS 0001-3-2 5.3.45
enum {
    OCH_PARAMETERS_ADD_NOT_ALLOWED = 0,
    OCH_PARAMETERS_ADD_ALLOWED = 1,
};

enum {
    OCH_PARAMETERS_MBN_CLASIC = 0,
    OCH_PARAMETERS_MBN_MULTI_BN = 1,
};

typedef struct {
    uint8_t add;
    uint8_t mbn;
} och_parameters_t;

/// PAS 0001-3-2 5.3.47
enum {
    ORGANISATION_NOT_SIGNIFICANT = 0xff,
    ORGANISATION_RESERVED_1 = 0xfe,
    ORGANISATION_RESERVED_2 = 0xfd,
    ORGANISATION_RESERVED_3 = 0xfc,
    ORGANISATION_RESERVED_4 = 0xfb,
};

/// PAS 0001-3-2 5.3.49
enum {
    REFERENCE_LIST_DC_COVERED = 0,
    REFERENCE_LIST_DC_UMBRELLA = 1,
};

enum {
    REFERENCE_LIST_CSO_NOT_ADVISE = 0,
    REFERENCE_LIST_CSO_ADVISE = 1,
};

enum {
    REFERENCE_LIST_CSG_NOT_ADVISE = 0,
    REFERENCE_LIST_CSG_ADVISE = 1,
};

typedef union {
    uint8_t _data;
    struct {
        unsigned int dc : 1;
        unsigned int cso : 1;
        unsigned int csg : 1;
        unsigned int _unused : 2;
        unsigned int revision : 3;
    };
} reference_list_t;

/// PAS 0001-3-2 5.3.52
enum {
    POLLING_NO = 0,
    POLLING_PROFILE_1 = 1,
    POLLING_PROFILE_2 = 2,
    POLLING_PROFILE_3 = 3,
    // other values reserved
};

typedef union {
    uint8_t _data;
    struct {
        unsigned int polling : 3;
        unsigned int cnt : 1;
        unsigned int iab : 1;
        unsigned int _zero : 3;
    };
} rt_data_info_t;

/// PAS 0001-3-2 5.3.56
typedef union {
    uint8_t _data;
    struct {
        unsigned int tra : 1;
        unsigned int ren : 1;
        unsigned int chg : 1;
        unsigned int pro : 1;
        unsigned int _reserved_0 : 1;
        unsigned int fix : 1;
        unsigned int _reserved_1 : 1;
        unsigned int _reserved_2 : 1;
    };
} rt_status_t;

/// PAS 0001-3-2 5.3.65
typedef union {
    uint8_t _data;
    struct {
        unsigned int version : 4;
        unsigned int network : 4;
    };
} system_id_t;

/// PAS 0001-3-2 5.3.71
enum {
    TRANS_MODE_PDR_HMSW = 1,
    TRANS_MODE_PDR_VMSW = 2,
    TRANS_MODE_UDP_MSG = 8,
    TRANS_MODE_UDP_TCP_APP = 9,
    // other values are reserved
};
typedef uint8_t trans_mode_t;

/// PAS 0001-3-2 5.3.75
enum {
    TYPE_NB_TYPE_END = 0,
    TYPE_NB_TYPE_TALK_GROUP = 1,
    TYPE_NB_TYPE_EMERGENCY = 2,
    TYPE_NB_TYPE_OPEN = 3,
};

typedef union {
    uint8_t _data;
    struct {
        unsigned int number : 6;
        unsigned int type : 2;
    };
} type_nb_t;

/// PAS 0001-3-2 5.3.76
enum {
    USER_PRIORITY_0 = 0,
    USER_PRIORITY_1 = 1,
    USER_PRIORITY_2 = 2,
    // values 3..15 are reserved
};

bool address_decode(address_t *address, const uint8_t **data_ptr);
void address_print(const address_t *address);

int address_list_decode(address_list_t **ptr_addrs, const uint8_t *data);
