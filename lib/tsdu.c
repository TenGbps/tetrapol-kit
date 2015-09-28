#define LOG_PREFIX "tsdu"
#include <tetrapol/log.h>
#include <tetrapol/tsdu.h>
#include <tetrapol/misc.h>
#include <tetrapol/bit_utils.h>
#include <tetrapol/phys_ch.h>
#include <tetrapol/misc.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_LEN(len, min_len, tsdu) \
    if ((len) < (min_len)) { \
        LOG(ERR, "%d data too short %d < %d", __LINE__, (len), (min_len)); \
        tsdu_destroy((tsdu_base_t *)tsdu); \
        return NULL; \
    }

static const char *codop_str[256] = {
    "N/A",                          // 0x00,
    "N/A",                          // 0x01,
    "N/A",                          // 0x02,
    "N/A",                          // 0x03,
    "N/A",                          // 0x04,
    "N/A",                          // 0x05,
    "N/A",                          // 0x06,
    "N/A",                          // 0x07,
    "D_REJECT",                     // 0x08,
    "D_REFUSAL",                    // 0x09,
    "U_END",                        // 0x0a,
    "D_BACK_CCH",                   // 0x0b,
    "D_RELEASE",                    // 0x0c,
    "U_ABORT",                      // 0x0d,
    "U_TERMINATE",                  // 0x0e,
    "D_HOOK_ON_INVITATION",         // 0x0f,
    "D_RETURN",                     // 0x10,
    "U_EVENT_REPORT",               // 0x11,
    "D_CALL_WAITING",               // 0x12,
    "D_AUTHENTICATION",             // 0x13,
    "U_AUTHENTICATION",             // 0x14,
    "N/A",                          // 0x15,
    "D_AUTHORISATION",              // 0x16,
    "U_ERROR_REPORT",               // 0x17,
    "D_CHANNEL_INIT",               // 0x18,
    "N/A",                          // 0x19,
    "N/A",                          // 0x1a,
    "N/A",                          // 0x1b,
    "N/A",                          // 0x1c,
    "N/A",                          // 0x1d,
    "N/A",                          // 0x1e,
    "N/A",                          // 0x1f,
    "U_REGISTRATION_REQ",           // 0x20,
    "D_REGISTRATION_NAK",           // 0x21,
    "D_REGISTRATION_ACK",           // 0x22,
    "D_FORCED_REGISTRATION",        // 0x23,
    "U_LOCATION_ACTIVITY",          // 0x24,
    "D_LOCATION_ACTIVITY_ACK",      // 0x25,
    "N/A",                          // 0x26,
    "N/A",                          // 0x27,
    "N/A",                          // 0x28,
    "N/A",                          // 0x29,
    "N/A",                          // 0x2a,
    "N/A",                          // 0x2b,
    "N/A",                          // 0x2c,
    "N/A",                          // 0x2d,
    "N/A",                          // 0x2e,
    "N/A",                          // 0x2f,
    "U_CALL_SETUP",                 // 0x30,
    "D_CALL_ALERT",                 // 0x31,
    "D_CALL_SETUP",                 // 0x32,
    "U_CALL_ANSWER",                // 0x33,
    "D_CALL_CONNECT",               // 0x34,
    "D_CALL_SWITCH",                // 0x35,
    "U_CALL_INTRUSION_PC",          // 0x36,
    "U_CALL_INTRUSION_OCH",         // 0x37,
    "N/A",                          // 0x38,
    "D_TRANSFER_NAK",               // 0x39,
    "U_TRANSFER_REQ",               // 0x3a,
    "U_CALL_INTRUSION_ECH",         // 0x3b,
    "U_CALL_RELEASE",               // 0x3c,
    "U_CALL_CONNECT",               // 0x3d,
    //"U_CALL_SWITCH",                // 0x3d,
    "D_CALL_START",                 // 0x3e,
    "N/A",                          // 0x3f,
    "N/A",                          // 0x40,
    "N/A",                          // 0x41,
    "D_FUNCTIONAL_SHORT_DATA",      // 0x42,
    "U_DATA_DOWN_ACCEPT",           // 0x43,
    "U_DATA_MSG_UP",                // 0x44,
    "D_DATA_MSG_DOWN",              // 0x45,
    "D_EXPLICIT_SHORT_DATA",        // 0x46,
    "X_UNKNOWN_47",                 // 0x47,
    "D_DATA_END",                   // 0x48,
    "D_DATAGRAM_NOTIFY",            // 0x49,
    "D_DATAGRAM",                   // 0x4a,
    "D_BROADCAST",                  // 0x4b,
    "D_DATA_SERV",                  // 0x4c,
    "N/A",                          // 0x4d,
    "D_DATA_DOWN_STATUS",           // 0x4e,
    "N/A",                          // 0x4f,
    "U_OCH_RELEASE",                // 0x50,
    "U_OCH_SETUP",                  // 0x51,
    "U_ECH_CLOSE",                  // 0x52,
    "D_EMERGENCY_NOTIFICATION",     // 0x53,
    "U_ECH_SETUP",                  // 0x54,
    "D_GROUP_ACTIVATION",           // 0x55,
    "D_ECH_ACTIVATION",             // 0x56,
    "D_GROUP_END",                  // 0x57,
    "D_GROUP_IDLE",                 // 0x58,
    "D_GROUP_REJECT",               // 0x59,
    "D_ECH_REJECT",                 // 0x5a,
    "D_GROUP_PAGING",               // 0x5b,
    "D_BROADCAST_NOTIFICATION",     // 0x5c,
    "D_CRISIS_NOTIFICATION",        // 0x5d,
    "N/A",                          // 0x5e,
    "D_EMERGENCY_ACK",              // 0x5f,
    "D_CONNECT_DCH",                // 0x60,
    "N/A",                          // 0x61,
    "D_CONNECT_CCH",                // 0x62,
    "D_DATA_AUTHENTICATION",        // 0x63,
    "D_DATA_REQUEST",               // 0x64,
    "D_DCH_OPEN",                   // 0x65,
    "U_DATA_REQUEST",               // 0x66,
    "D_EXTENDED_STATUS",            // 0x67,
    "D_CCH_OPEN",                   // 0x68,
    "D_BROADCAST_WAITING",          // 0x69,
    "N/A",                          // 0x6a,
    "N/A",                          // 0x6b,
    "N/A",                          // 0x6c,
    "N/A",                          // 0x6d,
    "N/A",                          // 0x6e,
    "N/A",                          // 0x6f,
    "D_ACCESS_DISABLED",            // 0x70,
    "D_TRAFFIC_ENABLED",            // 0x71,
    "D_TRAFFIC_DISABLED",           // 0x72,
    "U_DEVIATION_CLEAR",            // 0x73,
    "U_DEVIATION_SET",              // 0x74,
    "N/A",                          // 0x75,
    "D_DEVIATION_ON",               // 0x76,
    "D_ABILITY_MNGT",               // 0x77,
    "D_SERVICE_DISABLED",           // 0x78,
    "N/A",                          // 0x79,
    "N/A",                          // 0x7a,
    "N/A",                          // 0x7b,
    "N/A",                          // 0x7c,
    "N/A",                          // 0x7d,
    "N/A",                          // 0x7e,
    "N/A",                          // 0x7f,
    "D_EMERGENCY_NAK",              // 0x80,
    "U_EMERGENCY_REQ",              // 0x81,
    "D_GROUP_OVERLOAD_ID",          // 0x82,
    "D_ECH_OVERLOAD_ID",            // 0x83,
    "D_PRIORITY_GRP_WAITING",       // 0x84,
    "D_PRIORITY_GRP_ACTIVATION",    // 0x85,
    "D_OC_ACTIVATION",              // 0x86,
    "D_OC_REJECT",                  // 0x87,
    "D_OC_PAGING",                  // 0x88,
    "N/A",                          // 0x89,
    "N/A",                          // 0x8a,
    "N/A",                          // 0x8b,
    "N/A",                          // 0x8c,
    "N/A",                          // 0x8d,
    "N/A",                          // 0x8e,
    "N/A",                          // 0x8f,
    "D_SYSTEM_INFO",                // 0x90,
    "N/A",                          // 0x91,
    "D_GROUP_LIST",                 // 0x92,
    "D_GROUP_COMPOSITION",          // 0x93,
    "D_NEIGHBOURING_CELL",          // 0x94,
    "D_ECCH_DESCRIPTION",           // 0x95,
    "D_ADDITIONAL_PARTICIPANTS",    // 0x96,
    "X_UNKNOWN_97",                 // 0x97,
    "N/A",                          // 0x98,
    "N/A",                          // 0x99,
    "N/A",                          // 0x9a,
    "N/A",                          // 0x9b,
    "N/A",                          // 0x9c,
    "N/A",                          // 0x9d,
    "N/A",                          // 0x9e,
    "N/A",                          // 0x9f,
    "N/A",                          // 0xa0,
    "N/A",                          // 0xa1,
    "N/A",                          // 0xa2,
    "N/A",                          // 0xa3,
    "N/A",                          // 0xa4,
    "N/A",                          // 0xa5,
    "N/A",                          // 0xa6,
    "N/A",                          // 0xa7,
    "N/A",                          // 0xa8,
    "N/A",                          // 0xa9,
    "N/A",                          // 0xaa,
    "N/A",                          // 0xab,
    "N/A",                          // 0xac,
    "N/A",                          // 0xad,
    "N/A",                          // 0xae,
    "N/A",                          // 0xaf,
    "N/A",                          // 0xb0,
    "N/A",                          // 0xb1,
    "N/A",                          // 0xb2,
    "N/A",                          // 0xb3,
    "N/A",                          // 0xb4,
    "N/A",                          // 0xb5,
    "N/A",                          // 0xb6,
    "N/A",                          // 0xb7,
    "N/A",                          // 0xb8,
    "N/A",                          // 0xb9,
    "N/A",                          // 0xba,
    "N/A",                          // 0xbb,
    "N/A",                          // 0xbc,
    "N/A",                          // 0xbd,
    "N/A",                          // 0xbe,
    "N/A",                          // 0xbf,
    "N/A",                          // 0xc0,
    "N/A",                          // 0xc1,
    "N/A",                          // 0xc2,
    "N/A",                          // 0xc3,
    "N/A",                          // 0xc4,
    "D_INFORMATION_DELIVERY",       // 0xc5,
    "N/A",                          // 0xc6,
    "N/A",                          // 0xc7,
    "N/A",                          // 0xc8,
    "N/A",                          // 0xc9,
    "N/A",                          // 0xca,
    "N/A",                          // 0xcb,
    "N/A",                          // 0xcc,
    "N/A",                          // 0xcd,
    "N/A",                          // 0xce,
    "N/A",                          // 0xcf,
    "N/A",                          // 0xd0,
    "N/A",                          // 0xd1,
    "N/A",                          // 0xd2,
    "N/A",                          // 0xd3,
    "N/A",                          // 0xd4,
    "N/A",                          // 0xd5,
    "N/A",                          // 0xd6,
    "N/A",                          // 0xd7,
    "N/A",                          // 0xd8,
    "N/A",                          // 0xd9,
    "N/A",                          // 0xda,
    "N/A",                          // 0xdb,
    "N/A",                          // 0xdc,
    "N/A",                          // 0xdd,
    "N/A",                          // 0xde,
    "N/A",                          // 0xdf,
    "D_CALL_ACTIVATION",            // 0xe0,
    "D_CALL_COMPOSITION",           // 0xe1,
    "D_CALL_END",                   // 0xe2,
    "D_CALL_OVERLOAD_ID",           // 0xe3,
    "N/A",                          // 0xe4,
    "N/A",                          // 0xe5,
    "N/A",                          // 0xe6,
    "N/A",                          // 0xe7,
    "N/A",                          // 0xe8,
    "N/A",                          // 0xe9,
    "N/A",                          // 0xea,
    "N/A",                          // 0xeb,
    "N/A",                          // 0xec,
    "N/A",                          // 0xed,
    "N/A",                          // 0xee,
    "N/A",                          // 0xef,
    "N/A",                          // 0xf0,
    "N/A",                          // 0xf1,
    "N/A",                          // 0xf2,
    "N/A",                          // 0xf3,
    "N/A",                          // 0xf4,
    "N/A",                          // 0xf5,
    "N/A",                          // 0xf6,
    "N/A",                          // 0xf7,
    "N/A",                          // 0xf8,
    "N/A",                          // 0xf9,
    "N/A",                          // 0xfa,
    "N/A",                          // 0xfb,
    "N/A",                          // 0xfc,
    "N/A",                          // 0xfd,
    "N/A",                          // 0xfe,
    "N/A",                          // 0xff,
};

static const char *cause_str[256] = {
    // COMMON CAUSES
    "normal",                                            // 0x00
    "abnormal release",                                  // 0x01
    "terminal pre-emption",                              // 0x02
    "resource pre-emption",                              // 0x03
    "insufficient TCH quality",                          // 0x04
    "cleared by user",                                   // 0x05
    "power supply failure",                              // 0x06
    "application event",                                 // 0x07
    "identification error",                              // 0x08
    "unknown calling party",                             // 0x09
    "service barred calling party",                      // 0x0a
    "service barred called party",                       // 0x0b
    "software fault",                                    // 0x0c
    "service not implemented",                           // 0x0d
    "lack of resources",                                 // 0x0e
    "operator decision",                                 // 0x0f
    "protected call",                                    // 0x10
    "end of ringing",                                    // 0x11
    "voice inactivity",                                  // 0x12
    "host address not valid",                            // 0x13
    "Already forwarded",                                 // 0x14
    "inconsistent address",                              // 0x15
    "network event",                                     // 0x16
    "key error",                                         // 0x17
    "intrusion",                                         // 0x18
    "encryption error",                                  // 0x19
    "terminal not configured",                           // 0x1a
    "remote RT synchronisation",                         // 0x1b
    "coverage fault",                                    // 0x1c
    "unreachable master switch",                         // 0x1d
    "non authorised MOCH",                               // 0x1e
    "N/A",                                               // 0x1f
    "unknown TSDU",                                      // 0x20
    "missing mandatory IE",                              // 0x21
    "missing conditional IE",                            // 0x22
    "User erasure indication",                           // 0x23

    "N/A",                                               // 0x24
    "N/A",                                               // 0x25
    "N/A",                                               // 0x26
    "N/A",                                               // 0x27
    "N/A",                                               // 0x28
    "N/A",                                               // 0x29
    "N/A",                                               // 0x2a
    "N/A",                                               // 0x2b
    "N/A",                                               // 0x2c
    "N/A",                                               // 0x2d
    "N/A",                                               // 0x2e
    "N/A",                                               // 0x2f
    "N/A",                                               // 0x30
    "N/A",                                               // 0x31
    "N/A",                                               // 0x32
    "N/A",                                               // 0x33
    "N/A",                                               // 0x34
    "N/A",                                               // 0x35
    "N/A",                                               // 0x36
    "N/A",                                               // 0x37
    "N/A",                                               // 0x38
    "N/A",                                               // 0x39
    "N/A",                                               // 0x3a
    "N/A",                                               // 0x3b
    "N/A",                                               // 0x3c
    "N/A",                                               // 0x3d
    "N/A",                                               // 0x3e
    "N/A",                                               // 0x3f

    // PRIVATE CALL
    "no reply from called party",                        // 0x40
    "called party absent",                               // 0x41
    "called party busy",                                 // 0x42
    "unreachable remote terminal",                       // 0x43
    "unknown called user",                               // 0x44
    "double forwarding",                                 // 0x45
    "all called parties rejected",                       // 0x46
    "transfer failure",                                  // 0x47
    "user refusal",                                      // 0x48
    "called terminal not configured",                    // 0x49
    "address cannot be parsed",                          // 0x4a
    "unknown sub-address field",                         // 0x4b
    "PABX subscriber busy",                              // 0x4c
    "Internal TDX link fault",                           // 0x4d
    "external TDX link fault",                           // 0x4e
    "internal TDX link re-establishment",                // 0x4f
    "external TDX link re-establishment",                // 0x50
    "transfer",                                          // 0x51
    "silent call",                                       // 0x52
    "Called Party warned",                               // 0x53

    "N/A",                                               // 0x54
    "N/A",                                               // 0x55
    "N/A",                                               // 0x56
    "N/A",                                               // 0x57
    "N/A",                                               // 0x58
    "N/A",                                               // 0x59
    "N/A",                                               // 0x5a
    "N/A",                                               // 0x5b
    "N/A",                                               // 0x5c
    "N/A",                                               // 0x5d
    "N/A",                                               // 0x5e
    "N/A",                                               // 0x5f

    // DATA
    "buffer not empty",                                  // 0x60
    "UDT not connected",                                 // 0x61
    "downlink transfer",                                 // 0x62
    "message size does not match expected length",       // 0x63
    "application type error",                            // 0x64
    "message length error",                              // 0x65
    "encryption field error",                            // 0x66
    "priority error",                                    // 0x67
    "uplink data channel congestion",                    // 0x68
    "delivery time expired",                             // 0x69
    "uplink transfer priority",                          // 0x6a
    "incorrect transmission parameters",                 // 0x6b
    "incorrect low layer option",                        // 0x6c
    "transmission inactivity",                           // 0x6d
    "SDP applications not supported or not opened",      // 0x6e
    "key renewal",                                       // 0x6f

    "N/A",                                               // 0x70
    "N/A",                                               // 0x71
    "N/A",                                               // 0x72
    "N/A",                                               // 0x73
    "N/A",                                               // 0x74
    "N/A",                                               // 0x75
    "N/A",                                               // 0x76
    "N/A",                                               // 0x77
    "N/A",                                               // 0x78
    "N/A",                                               // 0x79
    "N/A",                                               // 0x7a
    "N/A",                                               // 0x7b
    "N/A",                                               // 0x7c
    "N/A",                                               // 0x7d
    "N/A",                                               // 0x7e
    "N/A",                                               // 0x7f

    // GROUP COMMUNICATION
    "open channel not created",                          // 0x80
    "open channel already set-up",                       // 0x81
    "unknown open channel",                              // 0x82
    "open channel cannot be set-up",                     // 0x83
    "coverage not guaranteed",                           // 0x84
    "open channel number not valid",                     // 0x85
    "cell out of coverage",                              // 0x86
    "maximum open channel duration reached",             // 0x87
    "maximum activation time reached",                   // 0x88
    "communication change",                              // 0x89
    "group already activated",                           // 0x8a
    "maximum OG exceeded",                               // 0x8b

    "N/A",                                               // 0x8c
    "N/A",                                               // 0x8d
    "N/A",                                               // 0x8e
    "N/A",                                               // 0x8f
    "N/A",                                               // 0x90
    "N/A",                                               // 0x91
    "N/A",                                               // 0x92
    "N/A",                                               // 0x93
    "N/A",                                               // 0x94
    "N/A",                                               // 0x95
    "N/A",                                               // 0x96
    "N/A",                                               // 0x97
    "N/A",                                               // 0x98
    "N/A",                                               // 0x99
    "N/A",                                               // 0x9a
    "N/A",                                               // 0x9b
    "N/A",                                               // 0x9c
    "N/A",                                               // 0x9d
    "N/A",                                               // 0x9e
    "N/A",                                               // 0x9f

    // EMERGENCY OPEN CHANNEL
    "emergency open channel call",                       // 0xa0

    "N/A",                                               // 0xa1
    "N/A",                                               // 0xa2
    "N/A",                                               // 0xa3
    "N/A",                                               // 0xa4
    "N/A",                                               // 0xa5
    "N/A",                                               // 0xa6
    "N/A",                                               // 0xa7
    "N/A",                                               // 0xa8
    "N/A",                                               // 0xa9
    "N/A",                                               // 0xaa
    "N/A",                                               // 0xab
    "N/A",                                               // 0xac
    "N/A",                                               // 0xad
    "N/A",                                               // 0xae
    "N/A",                                               // 0xaf

    // KEY
    "authentication error",                              // 0xb0
    "home switch access fault",                          // 0xb1

    "N/A",                                               // 0xb2
    "N/A",                                               // 0xb3
    "N/A",                                               // 0xb4
    "N/A",                                               // 0xb5
    "N/A",                                               // 0xb6
    "N/A",                                               // 0xb7
    "N/A",                                               // 0xb8
    "N/A",                                               // 0xb9
    "N/A",                                               // 0xba
    "N/A",                                               // 0xbb
    "N/A",                                               // 0xbc
    "N/A",                                               // 0xbd
    "N/A",                                               // 0xbe
    "N/A",                                               // 0xbf
    "N/A",                                               // 0xc0
    "N/A",                                               // 0xc1
    "N/A",                                               // 0xc2
    "N/A",                                               // 0xc3
    "N/A",                                               // 0xc4
    "N/A",                                               // 0xc5
    "N/A",                                               // 0xc6
    "N/A",                                               // 0xc7
    "N/A",                                               // 0xc8
    "N/A",                                               // 0xc9
    "N/A",                                               // 0xca
    "N/A",                                               // 0xcb
    "N/A",                                               // 0xcc
    "N/A",                                               // 0xcd
    "N/A",                                               // 0xce
    "N/A",                                               // 0xcf
    "N/A",                                               // 0xd0
    "N/A",                                               // 0xd1
    "N/A",                                               // 0xd2
    "N/A",                                               // 0xd3
    "N/A",                                               // 0xd4
    "N/A",                                               // 0xd5
    "N/A",                                               // 0xd6
    "N/A",                                               // 0xd7
    "N/A",                                               // 0xd8
    "N/A",                                               // 0xd9
    "N/A",                                               // 0xda
    "N/A",                                               // 0xdb
    "N/A",                                               // 0xdc
    "N/A",                                               // 0xdd
    "N/A",                                               // 0xde
    "N/A",                                               // 0xdf

    // REGISTRATION
    "RT not valid",                                      // 0xe0
    "inconsistent RT",                                   // 0xe1
    "unreachable HRSW",                                  // 0xe2
    "non-explicit address",                              // 0xe3
    "RT registration disabled",                          // 0xe4
    "SwMI database updating",                            // 0xe5
    "RT assigned to an attachment cell",                 // 0xe6
    "RT cannot be authenticated",                        // 0xe7
    "congestion",                                        // 0xe8
    "RSW saturation",                                    // 0xe9
    "MRSW saturation",                                   // 0xea
    "HRSW saturation",                                   // 0xeb
    "out of window",                                     // 0xec
    "RT registration filtered",                          // 0xed

    "N/A",                                               // 0xee
    "N/A",                                               // 0xef
    "N/A",                                               // 0xf0
    "N/A",                                               // 0xf1
    "N/A",                                               // 0xf2
    "N/A",                                               // 0xf3
    "N/A",                                               // 0xf4
    "N/A",                                               // 0xf5
    "N/A",                                               // 0xf6
    "N/A",                                               // 0xf7
    "N/A",                                               // 0xf8
    "N/A",                                               // 0xf9
    "N/A",                                               // 0xfa
    "N/A",                                               // 0xfb
    "N/A",                                               // 0xfc
    "N/A",                                               // 0xfd
    "N/A",                                               // 0xfe
    "N/A",                                               // 0xff
};

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

static void tsdu_base_print(const tsdu_base_t *tsdu)
{
    LOGF("\tCODOP=0x%02x (%s)\n\tPRIO=%d\n\tID_TSAP=%d\n",
            tsdu->codop, codop_str[tsdu->codop], tsdu->prio, tsdu->id_tsap);
    // TODO: print addr
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
        cell_id->rws_id = get_bits(4, data, 8);
    } else if (type == CELL_ID_FORMAT_1) {
        cell_id->bs_id = get_bits(4, data, 8);
        cell_id->rws_id = get_bits(6, data, 2);
    } else {
        LOG(WTF, "unknown cell_id_type (%d)", type);
        cell_id->bs_id = -1;
        cell_id->rws_id = -1;
    }
}

// specific for d_system_info - cell in offline mode
static void cell_id_decode2(cell_id_t *cell_id, const uint8_t *data)
{
    int type = get_bits(2, data, 8);
    if (type == CELL_ID_FORMAT_0) {
        cell_id->bs_id = get_bits(6, data, 10);
        cell_id->rws_id = get_bits(4, data, 4);;
    } else if (type == CELL_ID_FORMAT_1) {
        cell_id->bs_id = get_bits(4, data, 4);;
        cell_id->rws_id = get_bits(6, data, 10);;
    } else {
        LOG(WTF, "unknown cell_id_type (%d)", type);
        cell_id->bs_id = -1;
        cell_id->rws_id = -1;
    }
}

/// return true if another address follows, false otherwise
static bool address_decode(address_t *address, const uint8_t *data)
{
    const bool li = get_bits(1, data, 0);
    address->cna = get_bits(3, data, 1);

    switch (address->cna) {
        case ADDRESS_CNA_NOT_SIGNIFICANT:
            address->len = 0;
            if (get_bits(4, data, 4) != 0) {
                LOG(WTF, "CND == 0 but address = 0x%0x", get_bits(4, data, 4));
            }
            break;

        case ADDRESS_CNA_RFSI:
            address->len = 9;
            for (int i = 0; i < 9; ++i) {
                address->rfsi.addr[i] = get_bits(4, data , 4 + 4*i);
            }
            break;

        case ADDRESS_CNA_PABX:
            address->len = get_bits(4, data, 4);
            for (int i = 0; i < address->len; ++i) {
                address->pabx[i] = get_bits(4, data, 8 + 4*i);
            }
            break;

        case ADDRESS_CNA_X400:
        case ADDRESS_CNA_FUNCTIONAL:
        case ADDRESS_CNA_LONG:
        case ADDRESS_CNA_BINARY:
        case ADDRESS_CNA_ESCAPED_CODE:
            address->len = 0;
            LOG(ERR, "TODO: unsupported address CNA");
            break;

        default:
            address->len = 0;
            LOG(WTF, "unknown address CNA");
            break;
    }

    return li;
}

static void address_print(const address_t *address)
{
    LOGF("\t\tADDRESS CNA=%i ", address->cna);
    switch (address->cna) {
        case ADDRESS_CNA_NOT_SIGNIFICANT:
            LOGF("NOT_SIGNIFICANT\n");
            break;

        case ADDRESS_CNA_RFSI:
            LOGF("RFSI=%x%x%x-%x-%x%x-%x%x%x\n", address->rfsi.r[0],
                    address->rfsi.r[1], address->rfsi.r[2],
                    address->rfsi.f, address->rfsi.s[0],
                    address->rfsi.s[1], address->rfsi.i[0],
                    address->rfsi.i[1], address->rfsi.i[2]);
            break;

        case ADDRESS_CNA_PABX:
            LOGF("PABX=");
            for (int i = 0; i < address->len; ++i) {
                LOGF("%d", address->pabx[i]);
            }
            LOGF("\n");
            break;

        default:
            LOGF("TODO: (unsupported/unwknow)\n");
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

static void d_authentication_print(const tsdu_d_authentication_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    LOGF("\t\tKEY_REFERENCE: KEY_TYPE=%i KEY_INDEX=%i\n",
            tsdu->key_reference.key_type, tsdu->key_reference.key_index);
    char buf[3 * SIZEOF(tsdu_d_call_connect_t, valid_rt)];
    LOGF("\t\tVALID_RT=%s\n",
            sprint_hex(buf, tsdu->valid_rt, SIZEOF(tsdu_d_call_connect_t, valid_rt)));
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

static void d_authorisation_print(const tsdu_d_authorisation_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    if (tsdu->has_key_reference) {
        LOGF("\t\tKEY_REFERENCE: KEY_TYPE=%i KEY_INDEX=%i\n",
                tsdu->key_reference.key_type, tsdu->key_reference.key_index);
    }
}

static tsdu_d_forced_registration_t *
d_forced_registration_decode(const uint8_t *data, int len)
{
    tsdu_d_forced_registration_t *tsdu = tsdu_create(tsdu_d_forced_registration_t, 0);
    if (!tsdu) {
        return NULL;
    }
    CHECK_LEN(len, 9, tsdu);

    if (address_decode(&tsdu->calling_adr, &data[1])) {
        LOG(ERR, "Onli single address is supported in calling_adr");
    }
    return tsdu;
}

static void d_forced_registration_print(const tsdu_d_forced_registration_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    address_print(&tsdu->calling_adr);
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

static void d_group_activation_print(tsdu_d_group_activation_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    LOGF("\t\tACTIVATION_MODE: HOOK=%d TYPE=%d\n",
            tsdu->activation_mode.hook, tsdu->activation_mode.type);
    LOGF("\t\tGROUP_ID=%d\n", tsdu->group_id);
    LOGF("\t\tCOVERAGE_ID=%d\n", tsdu->coverage_id);
    LOGF("\t\tCHANNEL_ID=%d\n", tsdu->channel_id);
    LOGF("\t\tU_CH_SCRAMBLING=%d\n", tsdu->u_ch_scrambling);
    LOGF("\t\tD_CH_SCRAMBLING=%d\n", tsdu->d_ch_scrambling);
    LOGF("\t\tKEY_REFERENCE: KEY_TYPE=%i KEY_INDEX=%i\n",
            tsdu->key_reference.key_type, tsdu->key_reference.key_index);
    if (tsdu->has_addr_tti) {
        char buf[ADDR_PRINT_BUF_SIZE];
        LOGF("\t\tADDR_TTI=%s\n", addr_print(buf, &tsdu->addr_tti));
    }
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

static void d_group_list_print(tsdu_d_group_list_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    LOGF("\t\tREFERENCE_LIST REVISION=%d CSG=%d CSO=%d DC=%d\n",
            tsdu->reference_list.revision, tsdu->reference_list.csg,
            tsdu->reference_list.cso, tsdu->reference_list.dc);
    if (tsdu->reference_list.revision == 0) {
        return;
    }
    LOGF("\t\tINDEX_LIST MODE=%d INDEX=%d\n",
            tsdu->index_list.mode, tsdu->index_list.index);

    if (tsdu->nopen) {
        LOGF("\t\tOCH\n");
        for (int i = 0; i < tsdu->nopen; ++i) {
            LOGF("\t\t\tCOVERAGE_ID=%d CALL_PRIORITY=%d GROUP_ID=%d "
                    "OCH_PARAMETERS.ADD=%d OCH_PARAMETERS.MBN=%d "
                    "NEIGBOURING_CELL=%d\n",
                    tsdu->open[i].coverage_id,
                    tsdu->open[i].call_priority,
                    tsdu->open[i].group_id,
                    tsdu->open[i].och_parameters.add,
                    tsdu->open[i].och_parameters.mbn,
                    tsdu->open[i].neighbouring_cell);
        }
    }

    if (tsdu->ngroup) {
        LOGF("\t\tGROUP\n");
        for (int i = 0; i < tsdu->ngroup; ++i) {
            LOGF("\t\t\tCOVERAGE_ID=%d NEIGHBOURING_CALL=%d\n",
                    tsdu->group[i].coverage_id, tsdu->group[i].neighbouring_cell);
        }
    }

    if (tsdu->nemergency) {
        LOGF("\t\t\tEMERGENCY\n");
        for (int i = 0; i < tsdu->nemergency; ++i) {
            LOGF("\t\t\tCELL_ID.BS_ID=%d CELL_ID.RSW_ID=%d\n",
                    tsdu->emergency[i].cell_id.bs_id, tsdu->emergency[i].cell_id.rws_id);
        }
    }
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

static void d_group_composition_print(tsdu_d_group_composition_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    LOGF("\t\tGROUP_ID=%d\n", tsdu->group_id);
    for (int i = 0; i < tsdu->og_nb; ++i) {
        LOGF("\t\tGROUP_ID=%d\n", tsdu->group_ids[i]);
    }
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

addr_list_t *iei_adjacent_bn_list_decode(
        addr_list_t *adj_cells, const uint8_t *data, int len)
{
    int n = 0;
    if (adj_cells) {
        n = adj_cells->len;
    }
    n +=  len * 2 / 3;
    addr_list_t *p = realloc(adj_cells, sizeof(addr_list_t) + sizeof(addr_t[n]));
    if (!p) {
        LOG(ERR, "ERR OOM");
        return NULL;
    }
    if (!adj_cells) {
        p->len = 0;
    }
    adj_cells = p;

    for (int skip = 0; adj_cells->len < n; ++adj_cells->len) {
        addr_parse(&adj_cells->addrs[adj_cells->len], data, skip);
        skip += 12;
    }

    return adj_cells;
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
            addr_list_t *p = iei_adjacent_bn_list_decode(
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

static void d_neighbouring_cell_print(tsdu_d_neighbouring_cell_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    LOGF("\t\tCCR_CONFIG=%d\n", tsdu->ccr_config.number);
    if (!tsdu->ccr_config.number) {
        return;
    }
    LOGF("\t\tCCR_PARAM=%d\n", tsdu->ccr_param);
    for (int i = 0; i < tsdu->ccr_config.number; ++i) {
        LOGF("\t\t\tBN_NB=%d CHANNEL_ID=%d ADJACENT_PARAM=%d BN=%d LOC=%d EXP=%d RXLEV_ACCESS=%d\n",
                tsdu->adj_cells[i].bn_nb,
                tsdu->adj_cells[i].channel_id,
                tsdu->adj_cells[i].adjacent_param._data,
                tsdu->adj_cells[i].adjacent_param.bn,
                tsdu->adj_cells[i].adjacent_param.loc,
                tsdu->adj_cells[i].adjacent_param.exp,
                tsdu->adj_cells[i].adjacent_param.rxlev_access);
    }
    if (tsdu->cell_ids) {
        LOGF("\t\tCELL_IDs\n");
        for (int i = 0; i < tsdu->cell_ids->len; ++i) {
            LOGF("\t\t\tCELL_ID BS_ID=%d RSW_ID=%d\n",
                    tsdu->cell_ids->cell_ids[i].bs_id,
                    tsdu->cell_ids->cell_ids[i].rws_id);
        }
    }
    if (tsdu->cell_bns) {
        LOGF("\t\tCELL_BNs\n");
        for (int i = 0; i < tsdu->cell_bns->len; ++i) {
            char buf[ADDR_PRINT_BUF_SIZE];
            LOGF("\t\t\tCELL_BN=%s\n", addr_print(buf, &tsdu->cell_bns->addrs[i]));
        }
    }
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

static void d_system_info_print(tsdu_d_system_info_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    LOGF("\t\tCELL_STATE\n");
    LOGF("\t\t\tMODE=%03x\n", tsdu->cell_state.mode);
    if (tsdu->cell_state.mode == CELL_STATE_MODE_NORMAL) {
        LOGF("\t\t\tBCH=%d\n", tsdu->cell_state.bch);
        LOGF("\t\t\tROAM=%d\n", tsdu->cell_state.roam);
        LOGF("\t\t\tEXP=%d\n", tsdu->cell_state.exp);
        LOGF("\t\t\tRSERVED=%d\n", tsdu->cell_state._reserved_00);

        LOGF("\t\tCELL_CONFIG\n");
        LOGF("\t\t\tECCH=%d\n", tsdu->cell_config.eccch);
        LOGF("\t\t\tATTA=%d\n", tsdu->cell_config.atta);
        LOGF("\t\t\tRESERVED=%d\n", tsdu->cell_config._reserved_0);
        LOGF("\t\t\tMUX_TYPE=%d\n", tsdu->cell_config.mux_type);
        LOGF("\t\t\tSIM=%d\n", tsdu->cell_config.sim);
        LOGF("\t\t\tDC=%d\n", tsdu->cell_config.dc);
        LOGF("\t\tCOUNTRY_CODE=%d\n", tsdu->country_code);
        LOGF("\t\tSYSTEM_ID\n");
        LOGF("\t\t\tVERSION=%d\n", tsdu->system_id.version);
        LOGF("\t\t\tNETWORK=%d\n", tsdu->system_id.network);
        LOGF("\t\tLOC_AREA_ID\n");
        LOGF("\t\t\tLOC_ID=%d\n", tsdu->loc_area_id.loc_id);
        LOGF("\t\t\tMODE=%d\n", tsdu->loc_area_id.mode);
        LOGF("\t\tBN_ID=%d\n", tsdu->bn_id);
        LOGF("\t\tCELL_ID=%d%d%d-%d-%d\n",
                tsdu->cell_bn.r1,tsdu->cell_bn.r2,tsdu->cell_bn.r3,
                tsdu->cell_id.bs_id, tsdu->cell_id.rws_id);
        LOGF("\t\tU_CH_SCRAMBLING=%d\n", tsdu->u_ch_scrambling);
        LOGF("\t\tCELL_RADIO_PARAM\n");
        LOGF("\t\t\tTX_MAX=%d\n", tsdu->cell_radio_param.tx_max);
        LOGF("\t\t\tRADIO_LINK_TIMEOUT=%d\n",
                tsdu->cell_radio_param.radio_link_timeout);
        LOGF("\t\t\tPWR_TX_ADJUST=%d dBm\n",
                CELL_RADIO_PARAM_PWR_TX_ADJUST_TO_DBM[
                tsdu->cell_radio_param.pwr_tx_adjust]);
        LOGF("\t\t\tRX_LEV_ACCESS=%d dBm\n",
                CELL_RADIO_PARAM_RX_LEV_ACCESS_TO_DBM[
                tsdu->cell_radio_param.rx_lev_access]);
        LOGF("\t\tSYSTEM_TIME=%d\n", tsdu->system_time);
        LOGF("\t\tCELL_ACCESS\n");
        LOGF("\t\t\tMIN_SERVICE_CLASS=%d\n",
                tsdu->cell_access.min_service_class);
        LOGF("\t\t\tMIN_REG_CLASS=%d\n",
                tsdu->cell_access.min_reg_class);
        LOGF("\t\tSUPERFRAME_CPT=%d\n", tsdu->superframe_cpt);
    } else {
        LOGF("\t\tCELL_ID=%d%d%d-%d-%d\n",
                tsdu->cell_bn.r1,tsdu->cell_bn.r2,tsdu->cell_bn.r3,
                tsdu->cell_id.bs_id, tsdu->cell_id.rws_id);
        LOGF("\t\tU_CH_SCRAMBLING=%d\n", tsdu->u_ch_scrambling);
        LOGF("\t\tCELL_RADIO_PARAM\n");
        LOGF("\t\t\tTX_MAX=%d\n", tsdu->cell_radio_param.tx_max);
        LOGF("\t\t\tRADIO_LINK_TIMEOUT=%d\n",
                tsdu->cell_radio_param.radio_link_timeout);
        LOGF("\t\t\tPWR_TX_ADJUST=%d dBm\n",
                CELL_RADIO_PARAM_PWR_TX_ADJUST_TO_DBM[
                tsdu->cell_radio_param.pwr_tx_adjust]);
        LOGF("\t\t\tRX_LEV_ACCESS=%d dBm\n",
                CELL_RADIO_PARAM_RX_LEV_ACCESS_TO_DBM[
                tsdu->cell_radio_param.rx_lev_access]);
        LOGF("\t\tBAND=%d\n", tsdu->band);
        LOGF("\t\tCHANNEL_ID=%d\n", tsdu->channel_id);
    }
}

static tsdu_d_registration_nak_t *d_registration_nak_decode(const uint8_t *data, int len)
{
    tsdu_d_registration_nak_t *tsdu = tsdu_create(tsdu_d_registration_nak_t, 0);
    if (!tsdu) {
        return NULL;
    }

    CHECK_LEN(len, 10, tsdu);

    tsdu->cause                 = data[1];
    if (address_decode(&tsdu->host_adr, data + 4)) {
        LOG(ERR, "Only single address NAK is supported");
    }
    tsdu->bn_id                 = data[7];
    cell_id_decode1(&tsdu->cell_id, data + 8);

    return tsdu;
}

static void d_registration_nak_print(tsdu_d_registration_nak_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    LOGF("\t\tCAUSE=0x%02x (%s)\n", tsdu->cause, cause_str[tsdu->cause]);
    address_print(&tsdu->host_adr);
    LOGF("\t\tBN_ID=%d\n", tsdu->bn_id);
    LOGF("\t\tCELL_ID: BS_ID=%d RSW_ID=%d\n",
            tsdu->cell_id.bs_id, tsdu->cell_id.rws_id);
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
    if (address_decode(&tsdu->host_adr, data + 4)) {
        LOG(ERR, "Only single address ACK is supported");
    }
    tsdu->rt_min_registration   = data[9];
    tsdu->tlr_value             = data[10];
    tsdu->rt_data_info._data    = data[11];
    tsdu->group_id              = get_bits(12, data + 12, 0);

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

static void d_registration_ack_print(tsdu_d_registration_ack_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    LOGF("\t\tCOMPLETE_REG=%d\n", tsdu->complete_reg);
    LOGF("\t\tRT_MIN_ACTIVITY=%i\n", tsdu->rt_min_activity);
    LOGF("\t\tRT_STATUS FIX=%i PRO=%i CHG=%i REN=%i TRA=%i\n",
            tsdu->rt_status.fix, tsdu->rt_status.pro, tsdu->rt_status.chg,
            tsdu->rt_status.ren, tsdu->rt_status.tra);
    address_print(&tsdu->host_adr);
    LOGF("\t\tRT_MIN_REGISTRATION=%i\n", tsdu->rt_min_registration);
    LOGF("\t\tTLR_VALUE=%i\n", tsdu->tlr_value);
    LOGF("\t\tRT_DATA_INFO POLLING=%i CNT=%i IAB=%i\n",
            tsdu->rt_data_info.polling, tsdu->rt_data_info.cnt,
            tsdu->rt_data_info.iab);
    LOGF("\t\tGROUP_ID=%i\n", tsdu->group_id);
    if (tsdu->has_coverage_id) {
        LOGF("\t\tCOVERAGE_ID=%i\n", tsdu->coverage_id);
    }
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

static void d_connect_dch_print(tsdu_d_connect_dch_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    LOGF("\t\tDCH_LOW_LAYER=%d\n", tsdu->dch_low_layer);
    LOGF("\t\tCHANNEL_ID=%d\n", tsdu->channel_id);
    LOGF("\t\tU_CH_SCRAMBLING=%d\n", tsdu->u_ch_scrambling);
    LOGF("\t\tD_CH_SCRAMBLING=%d\n", tsdu->d_ch_scrambling);
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

static void d_return_print(tsdu_d_return_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    LOGF("\t\tCAUSE=0x%02x (%s)\n", tsdu->cause, cause_str[tsdu->cause]);
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

static void d_group_idle_print(tsdu_d_group_idle_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    LOGF("\t\tCAUSE=0x%02x (%s)\n", tsdu->cause, cause_str[tsdu->cause]);
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

static void d_ech_overload_id_print(const tsdu_d_ech_overload_id_t *tsdu)
{
    LOGF("\tCODOP=0x%0x (D_ECH_OVERLOAD_ID)\n", tsdu->base.codop);
    LOGF("\t\tACTIVATION_MODE: hook=%d type=%d\n",
            tsdu->activation_mode.hook, tsdu->activation_mode.type);
    LOGF("\t\tGROUP_ID=%d", tsdu->group_id);
    LOGF("\t\tCELL_ID: BS_ID=%d RSW_ID=%d\n",
            tsdu->cell_id.bs_id, tsdu->cell_id.rws_id);
    LOGF("\t\tORGANISATION=%d\n", tsdu->organisation);
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

static void d_unknown_print(const tsdu_unknown_codop_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    char buf[tsdu->len * 3 + 1];
    LOGF("\t\tUNKNOWN CODOP len=%d data=%s\n", tsdu->len,
            sprint_hex(buf, tsdu->data, tsdu->len));
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

static void d_data_end_print(const tsdu_d_data_end_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    LOGF("\t\tCAUSE=0x%02x (%s)\n", tsdu->cause, cause_str[tsdu->cause]);
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

static void d_datagram_notify_print(const tsdu_d_datagram_notify_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    LOGF("\t\tCALL_PRIORITY=%d\n", tsdu->call_priority);
    LOGF("\t\tMESSAGE_REFERENCE=%d\n", tsdu->message_reference);
    LOGF("\t\tKEY_REFERENCE: key_index=%d key_type=%d\n",
            tsdu->key_reference.key_index, tsdu->key_reference.key_type);
    if (tsdu->destination_port != -1) {
        LOGF("\t\tDESTINATION_PORT=%d\n", tsdu->destination_port);
    }
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

static void d_datagram_print(const tsdu_d_datagram_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    LOGF("\t\tCALL_PRIORITY=%d\n", tsdu->call_priority);
    LOGF("\t\tMESSAGE_REFERENCE=%d\n", tsdu->message_reference);
    LOGF("\t\tKEY_REFERENCE: key_type=%d key_index=%d\n",
            tsdu->key_reference.key_type, tsdu->key_reference.key_index);
    char buf[tsdu->len * 3 + 1];
    LOGF("\t\tDATA: len=%d data=%s\n", tsdu->len,
            sprint_hex(buf, tsdu->data, tsdu->len));
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

static void d_explicit_short_data_print(const tsdu_d_explicit_short_data_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    char buf[tsdu->len * 3 + 1];
    LOGF("\t\tDATA: len=%d data=%s\n", tsdu->len,
            sprint_hex(buf, tsdu->data, tsdu->len));
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

static void d_call_start_print(const tsdu_d_call_start_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    if (tsdu->has_key_reference) {
        LOGF("\t\tKEY_REFERENCE: KEY_TYPE=%i KEY_INDEX=%i\n",
                tsdu->key_reference.key_type, tsdu->key_reference.key_index);
    }
    if (tsdu->has_key_of_call) {
        char buf[sizeof(key_of_call_t) * 3];
        LOGF("\t\tKEY_OF_CALL: %s\n",
                sprint_hex(buf, tsdu->key_of_call, sizeof(key_of_call_t)));
    }
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

static void d_call_connect_print(const tsdu_d_call_connect_t *tsdu)
{
    tsdu_base_print(&tsdu->base);
    LOGF("\t\tCALL_TYPE ORIGIN=0x%x DESTINATION=0x%x TRFS=%d\n",
            tsdu->call_type.origin, tsdu->call_type.destination,
            tsdu->call_type.trfs);
    LOGF("\t\tCHANNEL_ID=%d\n", tsdu->channel_id);
    LOGF("\t\tU_CH_SCRAMBLING=%d\n", tsdu->u_ch_scrambling);
    LOGF("\t\tD_CH_SCRAMBLING=%d\n", tsdu->d_ch_scrambling);
    LOGF("\t\tKEY_REFERENCE: KEY_TYPE=%i KEY_INDEX=%i\n",
            tsdu->key_reference.key_type, tsdu->key_reference.key_index);
    char buf[3 * SIZEOF(tsdu_d_call_connect_t, valid_rt)];
    LOGF("\t\tVALID_RT=%s\n",
            sprint_hex(buf, tsdu->valid_rt, SIZEOF(tsdu_d_call_connect_t, valid_rt)));
    if (tsdu->has_key_of_call) {
        char buf[3 * sizeof(key_of_call_t)];
        LOGF("\t\tKEY_OF_CALL=%s\n",
                sprint_hex(buf, tsdu->key_of_call, sizeof(key_of_call_t)));
    }
}

int tsdu_d_decode(const uint8_t *data, int len, int prio, int id_tsap, tsdu_t **tsdu)
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
        case D_AUTHENTICATION:
            *tsdu = (tsdu_t *)d_authentication_decode(data, len);
            break;

        case D_AUTHORISATION:
            *tsdu = (tsdu_t *)d_authorisation_decode(data, len);
            break;

        case D_CALL_CONNECT:
            *tsdu = (tsdu_t *)d_call_connect_decode(data, len);
            break;

        case D_CALL_START:
            *tsdu = (tsdu_t *)d_call_start_decode(data, len);
            break;

        case D_DATA_END:
            *tsdu = (tsdu_t *)d_data_end_decode(data, len);
            break;

        case D_DATAGRAM:
            *tsdu = (tsdu_t *)d_datagram_decode(data, len);
            break;

        case D_DATAGRAM_NOTIFY:
            *tsdu = (tsdu_t *)d_datagram_notify_decode(data, len);
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

        case D_RETURN:
            *tsdu = (tsdu_t *)d_return_decode(data, len);
            break;

        case D_GROUP_IDLE:
            *tsdu = (tsdu_t *)d_group_idle_decode(data, len);
            break;

        default:
            *tsdu = (tsdu_t *)d_unknown_parse(data, len);
            LOG(WTF, "unsupported codop 0x%02x", codop);
    }

    if (*tsdu) {
        (*tsdu)->codop = codop;
        (*tsdu)->downlink = true;
        (*tsdu)->prio = prio;
        (*tsdu)->id_tsap = id_tsap;
    }

    return 0;
}

static void tsdu_d_print(const tsdu_t *tsdu)
{
    switch (tsdu->codop) {
        case D_AUTHENTICATION:
            d_authentication_print((const tsdu_d_authentication_t *)tsdu);
            break;

        case D_AUTHORISATION:
            d_authorisation_print((const tsdu_d_authorisation_t *)tsdu);
            break;

        case D_CALL_CONNECT:
            d_call_connect_print((const tsdu_d_call_connect_t *)tsdu);
            break;

        case D_CALL_START:
            d_call_start_print((const tsdu_d_call_start_t *)tsdu);
            break;

        case D_DATA_END:
            d_data_end_print((const tsdu_d_data_end_t *)tsdu);
            break;

        case D_DATAGRAM:
            d_datagram_print((const tsdu_d_datagram_t *)tsdu);
            break;

        case D_DATAGRAM_NOTIFY:
            d_datagram_notify_print((const tsdu_d_datagram_notify_t *)tsdu);
            break;

        case D_ECH_OVERLOAD_ID:
            d_ech_overload_id_print((const tsdu_d_ech_overload_id_t *)tsdu);
            break;

        case D_EXPLICIT_SHORT_DATA:
            d_explicit_short_data_print((const tsdu_d_explicit_short_data_t *)tsdu);
            break;

        case D_FORCED_REGISTRATION:
            d_forced_registration_print((const tsdu_d_forced_registration_t *)tsdu);
            break;

        case D_GROUP_ACTIVATION:
            d_group_activation_print((tsdu_d_group_activation_t *)tsdu);
            break;

        case D_GROUP_COMPOSITION:
            d_group_composition_print((tsdu_d_group_composition_t *)tsdu);
            break;

        case D_GROUP_LIST:
            d_group_list_print((tsdu_d_group_list_t *)tsdu);
            break;

        case D_NEIGHBOURING_CELL:
            d_neighbouring_cell_print((tsdu_d_neighbouring_cell_t *)tsdu);
            break;

        case D_SYSTEM_INFO:
            d_system_info_print((tsdu_d_system_info_t *)tsdu);
            break;

        case D_REGISTRATION_ACK:
            d_registration_ack_print((tsdu_d_registration_ack_t *)tsdu);
            break;

        case D_REGISTRATION_NAK:
            d_registration_nak_print((tsdu_d_registration_nak_t *)tsdu);
            break;

        case D_CONNECT_DCH:
            d_connect_dch_print((tsdu_d_connect_dch_t *)tsdu);
            break;

        case D_RETURN:
            d_return_print((tsdu_d_return_t *)tsdu);
            break;

        case D_GROUP_IDLE:
            d_group_idle_print((tsdu_d_group_idle_t *)tsdu);
            break;

        default:
            LOG(WTF, "Undefined downlink codop=0x%02x", tsdu->codop);

        case D_ABILITY_MNGT:
        case D_ACCESS_DISABLED:
        case D_ADDITIONAL_PARTICIPANTS:
        case D_BACK_CCH:
        case D_BROADCAST:
        case D_BROADCAST_NOTIFICATION:
        case D_BROADCAST_WAITING:
        case D_CALL_ACTIVATION:
        case D_CALL_ALERT:
        case D_CALL_COMPOSITION:
        case D_CALL_END:
        case D_CALL_OVERLOAD_ID:
        case D_CALL_SETUP:
        case D_CALL_SWITCH:
        case D_CALL_WAITING:
        case D_CCH_OPEN:
        case D_CONNECT_CCH:
        case D_CRISIS_NOTIFICATION:
        case D_DATA_AUTHENTICATION:
        case D_DATA_DOWN_STATUS:
        case D_DATA_MSG_DOWN:
        case D_DATA_REQUEST:
        case D_DATA_SERV:
        case D_DEVIATION_ON:
        case D_DCH_OPEN:
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
        case D_GROUP_PAGING:
        case D_GROUP_REJECT:
        case D_HOOK_ON_INVITATION:
        case D_CHANNEL_INIT:
        case D_INFORMATION_DELIVERY:
        case D_LOCATION_ACTIVITY_ACK:
        case D_OC_ACTIVATION:
        case D_OC_PAGING:
        case D_OC_REJECT:
        case D_PRIORITY_GRP_ACTIVATION:
        case D_PRIORITY_GRP_WAITING:
        case D_REFUSAL:
        case D_REJECT:
        case D_RELEASE:
        case D_SERVICE_DISABLED:
        case D_TRAFFIC_DISABLED:
        case D_TRAFFIC_ENABLED:
        case D_TRANSFER_NAK:
            LOG(WTF, "print not implemented: downlink codop=0x%02x",
                    tsdu->codop);
            d_unknown_print((tsdu_unknown_codop_t *)tsdu);
    }
}

static void tsdu_u_print(const tsdu_t *tsdu)
{
    switch (tsdu->codop) {
        default:
            LOG(WTF, "print not implemented: uplink codop=0x%02x",
                    tsdu->codop);
    }
}

void tsdu_print(tsdu_t *tsdu)
{
    if (tsdu->downlink) {
        tsdu_d_print(tsdu);
    } else {
        tsdu_u_print(tsdu);
    }
}
