#define LOG_PREFIX "data_frame"
#include <tetrapol/log.h>
#include <tetrapol/bit_utils.h>
#include <tetrapol/system_config.h>
#include <tetrapol/data_frame.h>
#include <tetrapol/misc.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum {
    FN_00 = 00,
    FN_01 = 01,
    FN_10 = 02,
    FN_11 = 03,
};

struct data_frame_priv_t {
    frame_t frames[SYS_PAR_DATA_FRAME_BLOCKS_MAX + 1];
    int fn[SYS_PAR_DATA_FRAME_BLOCKS_MAX + 1];
    int nframes;
    int nerrs;
};

data_frame_t *data_frame_create(void)
{
    data_frame_t *data_fr = malloc(sizeof(data_frame_t));
    if (!data_fr) {
        return NULL;
    }

    data_frame_reset(data_fr);

    return data_fr;
}

void data_frame_destroy(data_frame_t *data_fr)
{
    free(data_fr);
}

int data_frame_blocks(data_frame_t *data_fr)
{
    return data_fr->nframes;
}

void data_frame_reset(data_frame_t *data_fr)
{
    data_fr->nframes = 0;
    data_fr->nerrs = 0;
}

static bool check_parity(data_frame_t *data_fr)
{
    for (int i = 3; i < 3 + 64; ++i) {
        int parity = 0;
        for (int fr_no = 0; fr_no < data_fr->nframes; ++fr_no) {
            parity ^= data_fr->frames[fr_no].data.data[i];
        }
        if (parity) {
            return false;
        }
    }

    return true;
}

static void fix_by_parity(data_frame_t *data_fr)
{
    int err_fr_no = 0;

    for (int fr_no = 0; fr_no < data_fr->nframes; ++fr_no) {
        if (data_fr->frames[fr_no].broken) {
            err_fr_no = fr_no;
            break;
        }
    }

    // do not fix parity frame
    if (err_fr_no == data_fr->nframes - 1) {
        return;
    }

    for (int i = 1; i < 1 + 64 + 2; ++i) {
        int bit = 0;
        for (int fr_no = 0; fr_no < data_fr->nframes; ++fr_no) {
            if (fr_no != err_fr_no) {
                bit ^= data_fr->frames[fr_no].data.data[i];
            }
            data_fr->frames[err_fr_no].data.data[i] = bit;
        }
    }
}

static int data_frame_check_multiblock(data_frame_t *data_fr)
{
    if (data_fr->nerrs) {
        fix_by_parity(data_fr);
    } else {
        if (!check_parity(data_fr)) {
            LOG(ERR, "MB parity error %d", data_fr->nframes);
            data_frame_reset(data_fr);
            return -1;
        }
    }

    return 1;
}

static int data_frame_push_frame_(data_frame_t *data_fr, const frame_t *fr)
{
    const int r = data_frame_push_frame(data_fr, fr);

    switch (r) {
        case 0:
            return -1;
        case 1:
            return 2;
        default:
            return r;
    }
}

int data_frame_push_frame(data_frame_t *data_fr, const frame_t *fr)
{
    if (data_fr->nframes == ARRAY_LEN(data_fr->frames)) {
        data_frame_reset(data_fr);
    }

    data_fr->nerrs += fr->broken ? 1 : 0;

    if (data_fr->nerrs > 1) {
        data_frame_reset(data_fr);
        return -1;
    }

    const int fn = fr->data.data[0] | (fr->data.data[1] << 1);
    data_fr->fn[data_fr->nframes] = fr->broken ? -1 : fn;

    memcpy(&data_fr->frames[data_fr->nframes], fr, sizeof(frame_t));
    ++data_fr->nframes;

    // single frame
    if (data_fr->nframes == 1) {
        if (fr->broken) {
            return 0;
        }
        if (fn == FN_00) {
            return 1;
        }
        if (fn != FN_01) {
            LOG(DBG, "MB err");
            data_frame_reset(data_fr);
            return -1;
        }
        return 0;
    }

    const int fn_prev = data_fr->fn[data_fr->nframes - 2];
    const bool fr_errors_prev = data_fr->frames[data_fr->nframes - 2].broken;

    // check for dualframe or multiframe
    if (data_fr->nframes == 2) {
        if (fr->broken) {
            if (fn_prev != FN_01) {
                LOG(DBG, "MB err");
                data_frame_reset(data_fr);
                return -1;
            }
            return 0;
        }
        if (fn == FN_11) {
            if (fr_errors_prev) {
                LOG(DBG, "MB err");
                data_frame_reset(data_fr);
                return -1;
            }
            return 1;
        }
        if (fn != FN_10) {
            LOG(DBG, "MB err");
            data_frame_reset(data_fr);
            return data_frame_push_frame_(data_fr, fr);
        }
        return 0;
    }

    // check multiframe, inner frames
    if (data_fr->nframes == 3) {
        if (fr->broken) {
            return 0;
        }
        if (fn != FN_10 && fn != FN_11) {
            LOG(DBG, "MB err");
            data_frame_reset(data_fr);
            return data_frame_push_frame_(data_fr, fr);
        }
        return 0;
    }

    // end of multiframe, final frame is invalid
    if (fr->broken) {
        if (fn_prev == FN_10) {
            return data_frame_check_multiblock(data_fr);
        }
        return 0;
    }

    if (fn == FN_11) {
        if (fn_prev != FN_11 && !fr_errors_prev) {
            LOG(DBG, "MB err");
            data_frame_reset(data_fr);
            return -1;
        }
        return 0;
    }

    // check multiframe, pre-end of multiframe
    if (fn == FN_10) {
        if (fn_prev != FN_11 && !fr_errors_prev) {
            LOG(DBG, "MB err");
            data_frame_reset(data_fr);
            return -1;
        }
        return 0;
    }

    if (fn == FN_01) {
        if (fn_prev != FN_10 && !fr_errors_prev) {
            LOG(DBG, "MB err");
            data_frame_reset(data_fr);
            return -1;
        }
        return data_frame_check_multiblock(data_fr);
    }

    LOG(DBG, "MB err");
    data_frame_reset(data_fr);
    return data_frame_push_frame_(data_fr, fr);
}

int data_frame_get_bytes(data_frame_t *data_fr, uint8_t *data)
{
    const int nframes = (data_fr->nframes <= 2) ?
        data_fr->nframes : data_fr->nframes - 1;

    memset(data, 0, 8*nframes);
    for (int fr_no = 0; fr_no < nframes; ++fr_no) {
        pack_bits(data, data_fr->frames[fr_no].data.data + 2, 64*fr_no, 64);
    }

    data_frame_reset(data_fr);

    return nframes * 64;
}
