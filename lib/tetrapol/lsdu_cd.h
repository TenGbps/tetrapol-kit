#pragma once

#include <tetrapol/msg_coding.h>

#include <stdbool.h>
#include <stdint.h>

/**
  PAS 0001-3-4 6.4.3
  X_UNKNOWN_xx messages float in air but are not part of public specification
  */
enum {
    // 0x00 reserved
    TP_ADDRESS                  = 0x01,
    TP_ALIAS                    = 0x02,
    AMBIENCE_LISTENING          = 0x03,
    TP_ADDRESS_CHIF             = 0x04,
    TP_ADDRES_INHIB             = 0x05,
    TP_ADDRESS_CHIF_INHIB       = 0x06,
};

typedef uint8_t codop_t;

/// PAS 0001-3-4 6.4.2.1
typedef struct {
    uint8_t codop;
    uint8_t signature : 4;
    uint8_t modifier_number : 4;
    address_t rt_address;
    uint8_t _stuffing_len;
    uint8_t _stuffing[8];
} lsdu_cd_tp_address_t;

typedef lsdu_cd_tp_address_t lsdu_cd_tp_addres_inhib_t;

/// PAS 0001-3-4 6.4.2.2
typedef struct {
    uint8_t codop;
    uint8_t signature : 4;
    uint8_t modifier_number : 4;
    uint8_t rt_alias[9];
} lsdu_cd_tp_alias_t;

/// PAS 0001-3-4 6.4.2.3
typedef struct {
    uint8_t codop;
    uint8_t transmit_priority : 4;
    uint8_t listen_mode : 4;
    uint8_t called_addr[5];
} lsdu_cd_ambience_listening_t;

/*
// ** Those are not defined in accessible PAS

/// PAS 0001-3-4 6.4.2.4
typedef struct {
    uint8_t codop;
} lsdu_cd_tp_address_chif_t;

/// PAS 0001-3-4 6.4.2.6
typedef struct {
    uint8_t codop;
} lsdu_cd_tp_address_chif_inhib_t;
*/

typedef struct {
    codop_t codop;
    // PAS 0001-3-4 6.2 lengh of LSDU is limited to 59 bytes (-1 for codop)
    uint8_t data[58];
    uint8_t len;
} lsdu_cd_unknown_t;

// do not use directly, this struct must be first member of each LSDU structure
typedef union {
    lsdu_cd_unknown_t unknown;
    lsdu_cd_tp_address_t tp_address;
    lsdu_cd_tp_addres_inhib_t tp_addres_inhib;
    lsdu_cd_tp_alias_t tp_alias;
    lsdu_cd_ambience_listening_t ambience_listening;
    //lsdu_cd_tp_address_chif_t tp_address_chif;
    //lsdu_cd_tp_address_chif_inhib_t tp_address_chif_inhib;
} lsdu_cd_t;

/**
 * @brief lsdu_d_decode Compose LSDU from TPDUs.
 * @param data LSDU data.
 * @param len length of data in bytes
 * @param lsdu Allocated lsdu_cd_t where result is stored.
 * @return 0 on success, -1 on fail.
 */
int lsdu_cd_decode(const uint8_t *data, int len, lsdu_cd_t **lsdu);

void lsdu_cd_print(const lsdu_cd_t *lsdu);
void lsdu_cd_destroy(lsdu_cd_t *lsdu);
