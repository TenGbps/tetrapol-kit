#define LOG_PREFIX "pch"
#include <tetrapol/log.h>
#include <tetrapol/pch.h>
#include <tetrapol/addr.h>
#include <tetrapol/misc.h>
#include <tetrapol/data_frame.h>

#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t act_bitmap[8];
    uint8_t naddrs;
    addr_t addrs[4];
} pch_data_t;

struct pch_priv_t {
    data_frame_t *data_fr;
    pch_data_t pch_data;
    tpol_t *tpol;
};

pch_t *pch_create(tpol_t *tpol)
{
    pch_t *pch = malloc(sizeof(pch_t));
    if (!pch) {
        return NULL;
    }

    pch->data_fr = data_frame_create();
    if (!pch->data_fr) {
        free(pch);
        return NULL;
    }

    pch->tpol = tpol;

    return pch;
}

void pch_destroy(pch_t *pch)
{
    if (pch) {
        data_frame_destroy(pch->data_fr);
    }
    free(pch);
}

void pch_reset(pch_t *pch)
{
    data_frame_reset(pch->data_fr);
}

bool pch_push_frame(pch_t *pch, const frame_t *fr)
{
    if (data_frame_push_frame(pch->data_fr, fr) <= 0) {
        if (pch->tpol->frame_no % 2) {
            LOG(DBG, "PCH frame broken");
            // TODO: PCH block lost
        }
        return false;
    }

    if (data_frame_blocks(pch->data_fr) != 2) {
        LOG(WTF, "invalid number of blocks for PCH %d\n",
                data_frame_blocks(pch->data_fr));
        return false;
    }

    uint8_t data[(2 * 92) / 8];  // (2 * 64) / 8 should be OK, but ...
    const int size = data_frame_get_bytes(pch->data_fr, data);
    if (size != 2*64) {
        LOG(WTF, "block size: %d != 128\n", size);
        return false;
    }

    memcpy(pch->pch_data.act_bitmap, data, sizeof(pch->pch_data.act_bitmap));

    pch->pch_data.naddrs = 0;
    for (int i = 0; i < ARRAY_LEN(pch->pch_data.addrs); ++i) {
        addr_t *addr = &pch->pch_data.addrs[pch->pch_data.naddrs];
        addr_parse(addr, data + 8 + 2*i, 0);
        if (!addr_is_tti_no_st(addr, false)) {
            ++pch->pch_data.naddrs;
        }
    }

    return true;
}

void pch_print(pch_t *pch)
{
    char buf[ARRAY_LEN(pch->pch_data.act_bitmap) * 3];
    LOGF("PCH: activation_bitmap=%s\n",
            sprint_hex(buf, pch->pch_data.act_bitmap,
                ARRAY_LEN(pch->pch_data.act_bitmap)));
    for (int i = 0; i < pch->pch_data.naddrs; ++i) {
        LOGF("\taddr %d: %s\n", i, addr_print(buf, &pch->pch_data.addrs[i]));
    }
}

// TODO: Add tick method and report pagging of stations. When stations is in
// list of pch_data.naddrs link is lost on downlink and all existing
// connections should be closed.

