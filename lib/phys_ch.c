#define LOG_PREFIX "phys_ch"

#include <tetrapol/tetrapol_int.h>
#include <tetrapol/log.h>
#include <tetrapol/frame_json.h>
#include <tetrapol/system_config.h>
#include <tetrapol/tsdu.h>
#include <tetrapol/misc.h>
#include <tetrapol/phys_ch.h>
#include <tetrapol/tp_timer.h>
#include <tetrapol/frame.h>
#include <tetrapol/cch.h>
#include <tetrapol/tch.h>

#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

// max error rate for 2 frame synchronization sequences
#define MAX_FRAME_SYNC_ERR 1

#define DATA_OFFS (FRAME_LEN/2)

struct phys_ch_priv_t {
    int band;           ///< VHF or UHF
    uint8_t dir;        ///< direction (downlink / uplink)
    int radio_ch_type;  ///< control or traffic
    int sync_errs;      ///< cumulative no. of errors in frame synchronisation
    bool has_frame_sync;
    int scr;            ///< SCR, scrambling constant
    int scr_last;       ///< SCR, used to detech if SRC changes
    int scr_guess;      ///< SCR with best score when guessing SCR
    int scr_confidence; ///< required confidence for SCR detection
    int scr_stat[128];  ///< statistics for SCR detection
    uint8_t *data_begin;    ///< start of unprocessed part of data
    uint8_t *data_end;      ///< end of unprocessed part of data
    uint8_t data[10*FRAME_LEN];
    frame_decoder_t *fd;
    // CCH specific data, will be union with traffich CH specicic data
    tp_timer_t *tp_timer;
    cch_t *cch;
    tch_t *tch;
    tpol_t *tpol;
};

static int process_frame(phys_ch_t *phys_ch, const uint8_t *fr_data);

phys_ch_t *tetrapol_phys_ch_create(tetrapol_t *tetrapol)
{
    const tetrapol_cfg_t *cfg = tetrapol_get_cfg(tetrapol);

    phys_ch_t *phys_ch = calloc(1, sizeof(phys_ch_t));
    if (phys_ch == NULL) {
        return NULL;
    }

    phys_ch->tpol = tetrapol_get_tpol(tetrapol);
    phys_ch->band = cfg->band;
    phys_ch->dir = cfg->dir;
    phys_ch->radio_ch_type = cfg->radio_ch_type;
    phys_ch->data_begin = phys_ch->data_end = phys_ch->data + DATA_OFFS;
    phys_ch->tpol->rx_offs = 0;
    phys_ch->tpol->frame_no = FRAME_NO_UNKNOWN;
    phys_ch->scr = PHYS_CH_SCR_DETECT;
    phys_ch->scr_last = PHYS_CH_SCR_DETECT;
    phys_ch->scr_confidence = 50;
    phys_ch->tp_timer = tp_timer_create();

    phys_ch->fd = frame_decoder_create(cfg->band, 0, FRAME_TYPE_AUTO);
    if (!phys_ch->fd) {
        tp_timer_destroy(phys_ch->tp_timer);
        free(phys_ch);
        return NULL;
    }

    if (cfg->radio_ch_type == TETRAPOL_RADIO_CCH) {
        phys_ch->cch = cch_create(phys_ch->tpol);
        if (phys_ch->cch) {
            tp_timer_register(phys_ch->tp_timer, cch_tick, phys_ch->cch);
            return phys_ch;
        }
    }

    if (cfg->radio_ch_type == TETRAPOL_RADIO_TCH) {
        phys_ch->tch = tch_create(phys_ch->tpol);
        if (phys_ch->tch) {
            tp_timer_register(phys_ch->tp_timer, tch_tick, phys_ch->tch);
            return phys_ch;
        }
    }

    frame_decoder_destroy(phys_ch->fd);
    tp_timer_destroy(phys_ch->tp_timer);
    free(phys_ch);

    return NULL;
}

void tetrapol_phys_ch_destroy(phys_ch_t *phys_ch)
{
    if (phys_ch->radio_ch_type == TETRAPOL_RADIO_CCH) {
        cch_destroy(phys_ch->cch);
    }
    if (phys_ch->radio_ch_type == TETRAPOL_RADIO_TCH) {
        tch_destroy(phys_ch->tch);
    }
    frame_decoder_destroy(phys_ch->fd);
    tp_timer_destroy(phys_ch->tp_timer);
    free(phys_ch);
}

int tetrapol_phys_ch_get_scr(phys_ch_t *phys_ch)
{
    return phys_ch->scr;
}

void tetrapol_phys_ch_set_scr(phys_ch_t *phys_ch, int scr)
{
    phys_ch->scr = scr;
    memset(&phys_ch->scr_stat, 0, sizeof(phys_ch->scr_stat));
}

int tetrapol_phys_ch_get_scr_confidence(phys_ch_t *phys_ch)
{
    return phys_ch->scr_confidence;
}

void tetrapol_phys_ch_set_scr_confidence(
        phys_ch_t *phys_ch, int scr_confidence)
{
    phys_ch->scr_confidence = scr_confidence;
}

static uint8_t differential_dec(uint8_t *data, int size, uint8_t first_bit)
{
    while (size--) {
        first_bit = *data = *data ^ first_bit;
        ++data;
    }
    return first_bit;
}

int tetrapol_phys_ch_recv(phys_ch_t *phys_ch, uint8_t *buf, int len)
{
    const int data_len = phys_ch->data_end - phys_ch->data_begin;

    memmove(phys_ch->data, phys_ch->data_begin - DATA_OFFS, data_len + DATA_OFFS);
    phys_ch->data_begin = phys_ch->data + DATA_OFFS;
    phys_ch->data_end = phys_ch->data_begin + data_len;

    const int space = sizeof(phys_ch->data) - data_len - DATA_OFFS;
    len = (len > space) ? space : len;

    memcpy(phys_ch->data_end, buf, len);
    phys_ch->data_end += len;

    if (phys_ch->dir == DIR_UPLINK) {
        for (uint8_t *b = phys_ch->data_end - len; b < phys_ch->data_end; ++b) {
            *b ^= 0x01;
        }
    }

    return len;
}

// compare bite stream to differentialy encoded synchronization sequence
static int cmp_frame_sync(const uint8_t *data)
{
    const uint8_t frame_dsync[] = { 1, 0, 1, 0, 0, 1, 1, };
    int sync_err = 0;
    for(int i = 0; i < sizeof(frame_dsync); ++i) {
        sync_err += frame_dsync[i] ^ data[i + 1];
    }
    return sync_err;
}

/**
  Find 2 consecutive frame synchronization sequences.

  Using raw stream (before differential decoding) simplyfies search
  because only signal polarity must be considered,
  there is lot of troubles with error handlig after differential decoding.
  */
static int find_frame_sync(phys_ch_t *phys_ch)
{
    const uint8_t *end = phys_ch->data_end - FRAME_LEN - FRAME_HDR_LEN;
    int sync_err = MAX_FRAME_SYNC_ERR + 1;
    while (phys_ch->data_begin <= end) {
        sync_err = cmp_frame_sync(phys_ch->data_begin) +
            cmp_frame_sync(phys_ch->data_begin + FRAME_LEN);
        if (sync_err <= MAX_FRAME_SYNC_ERR) {
            break;
        }

        ++phys_ch->data_begin;
        ++phys_ch->tpol->rx_offs;
    }

    if (sync_err <= MAX_FRAME_SYNC_ERR) {
        phys_ch->sync_errs = 0;
        return 1;
    }

    return 0;
}

static void copy_frame_data(phys_ch_t *phys_ch, uint8_t *fr_data)
{
    memcpy(fr_data, phys_ch->data_begin + FRAME_HDR_LEN, FRAME_DATA_LEN);
    phys_ch->data_begin += FRAME_LEN;
    phys_ch->tpol->rx_offs += FRAME_LEN;

    differential_dec(fr_data, FRAME_DATA_LEN, 0);
}

/// return number of acquired frames (0 or 1) or -1 on error
static int get_frame(phys_ch_t *phys_ch, uint8_t *fr_data)
{
    if (phys_ch->data_end - phys_ch->data_begin < FRAME_LEN) {
        return 0;
    }

    // are we in sync?
    if (cmp_frame_sync(phys_ch->data_begin) == 0) {
        copy_frame_data(phys_ch, fr_data);
        if (phys_ch->sync_errs > 0) {
            --phys_ch->sync_errs;
        }
        return 1;
    }

    if (phys_ch->sync_errs > 6) {
        LOG(INFO, "get_frame() - sync lost sync_errs=%d", phys_ch->sync_errs);
        return -1;
    }

    // look for synchoronization pattern shifted by some offset from expected
    // possition. At the same time look for synchronization pattern of the
    // following frame. If pattern(s) are found, synchronization is restored.
    int sync_errs1 = INT_MAX;
    int sync_errs2 = INT_MAX;
    const uint8_t *end = phys_ch->data_end - FRAME_LEN - FRAME_HDR_LEN;
    uint8_t *data = phys_ch->data_begin;
    uint8_t *rdata = phys_ch->data_begin;
    uint8_t *sync_pos1 = NULL;
    uint8_t *sync_pos2 = NULL;
    for (int i = 0; i < DATA_OFFS; ++i) {
        if (data > end) {
            return 0;
        }

        int e = cmp_frame_sync(data);
        if (e < sync_errs1) {
            sync_pos1 = data;
            sync_errs1 = e;
        }

        e = cmp_frame_sync(rdata);
        if (e < sync_errs1) {
            sync_pos1 = rdata;
            sync_errs1 = e;
        }

        e = cmp_frame_sync(data + FRAME_LEN);
        if (e < sync_errs2) {
            sync_pos2 = data;
            sync_errs2 = e;
        }

        e = cmp_frame_sync(rdata + FRAME_LEN);
        if (e < sync_errs2) {
            sync_pos2 = rdata;
            sync_errs2 = e;
        }

        if (sync_errs1 == 0 || sync_errs2 == 0) {
            break;
        }

        ++data;
        --rdata;
    }

    // increase error counter only if we have not found 2 consecutive sync patterns
    if (sync_errs1 != 0 || sync_errs2 != 0 || sync_pos1 != sync_pos2) {
        phys_ch->sync_errs = 2 * phys_ch->sync_errs + 2;
    }

    if (phys_ch->sync_errs > 10) {
        LOG(INFO, "get_frame() sync lost sync_errs=%d", phys_ch->sync_errs);
        return -1;
    }

    uint8_t *sync_pos = (sync_errs1 < sync_errs2) ? sync_pos1 : sync_pos2;
    phys_ch->tpol->rx_offs += sync_pos - phys_ch->data_begin;
    phys_ch->data_begin = sync_pos;

    copy_frame_data(phys_ch, fr_data);
    LOG(INFO, "get_frame() sync fail sync_errs=%d", phys_ch->sync_errs);

    return 1;
}

int tetrapol_phys_ch_process(phys_ch_t *phys_ch)
{
    if (!phys_ch->has_frame_sync) {
        int n = phys_ch->data_end - phys_ch->data_begin;
        phys_ch->has_frame_sync = find_frame_sync(phys_ch);
        n -= phys_ch->data_end - phys_ch->data_begin;
        if (!phys_ch->has_frame_sync) {
            tp_timer_tick(phys_ch->tp_timer, true, n * 20000 / 160);
            return 0;
        }
        LOG(INFO, "Frame sync found");
        phys_ch->tpol->frame_no = FRAME_NO_UNKNOWN;
        if (phys_ch->cch) {
            cch_fr_error(phys_ch->cch);
        }
    }

    int r = 1;
    uint8_t fr_data[FRAME_DATA_LEN];
    while ((r = get_frame(phys_ch, fr_data)) > 0) {
        process_frame(phys_ch, fr_data);
        tp_timer_tick(phys_ch->tp_timer, false, 20000);
        if (phys_ch->tpol->frame_no != FRAME_NO_UNKNOWN) {
            phys_ch->tpol->frame_no = (phys_ch->tpol->frame_no + 1) % 200;
        }
    }

    if (r == 0) {
        return 0;
    }

    LOG(INFO, "Frame sync lost");
    phys_ch->has_frame_sync = false;

    return 0;
}

/**
  Try detect (and set) SCR - scrambling constant.

  @return SCR wich have now best score
  */
static void detect_scr(phys_ch_t *phys_ch, const uint8_t *fr_data)
{
    // compute SCR statistics
    for(int scr = 0; scr < ARRAY_LEN(phys_ch->scr_stat); ++scr) {
        frame_t fr;
        frame_decoder_reset(phys_ch->fd, phys_ch->band, scr, FRAME_TYPE_AUTO);
        frame_decoder_decode(phys_ch->fd, &fr, fr_data);
        if (fr.broken) {
            phys_ch->scr_stat[scr] -= 2;
            if (phys_ch->scr_stat[scr] < 0) {
                phys_ch->scr_stat[scr] = 0;
            }
            continue;
        }

        ++phys_ch->scr_stat[scr];
    }

    // get difference in statistic for two best SCRs
    // and check best SCR confidence
    int scr_max = 0, scr_max2 = 1;
    if (phys_ch->scr_stat[0] < phys_ch->scr_stat[1]) {
        scr_max = 1;
        scr_max2 = 0;
    }
    for(int scr = 2; scr < ARRAY_LEN(phys_ch->scr_stat); ++scr) {
        if (phys_ch->scr_stat[scr] >= phys_ch->scr_stat[scr_max]) {
            scr_max2 = scr_max;
            scr_max = scr;
        }
    }
    if (phys_ch->scr_stat[scr_max] - phys_ch->scr_confidence > phys_ch->scr_stat[scr_max2]) {
        tetrapol_phys_ch_set_scr(phys_ch, scr_max);
        LOG(INFO, "SCR detected %d", scr_max);
    }

    phys_ch->scr_guess = scr_max;
}

static int process_frame(phys_ch_t *phys_ch, const uint8_t *fr_data)
{
    if (phys_ch->scr == PHYS_CH_SCR_DETECT) {
        detect_scr(phys_ch, fr_data);
    }

    const int scr = (phys_ch->scr == PHYS_CH_SCR_DETECT) ?
        phys_ch->scr_guess : phys_ch->scr;

    if (phys_ch->scr_last != scr) {
        printf("{ \"event\": \"scr\", \"scr\": %d }\n", scr);
        phys_ch-> scr_last = scr;
    }

    const int fr_type = (phys_ch->radio_ch_type == TETRAPOL_RADIO_CCH) ?
        FRAME_TYPE_DATA : FRAME_TYPE_AUTO;

    frame_t fr;
    frame_decoder_reset(phys_ch->fd, phys_ch->band, scr, fr_type);
    frame_decoder_decode(phys_ch->fd, &fr, fr_data);

    if (!fr.broken) {
        frame_json(phys_ch->tpol, &fr);
    }

    if (phys_ch->radio_ch_type == TETRAPOL_RADIO_CCH) {
        // TODO: report when frame_no is detected
        return cch_push_frame(phys_ch->cch, &fr);
    }

    if (!tch_push_frame(phys_ch->tch, &fr)) {
        return 0;
    }

    // HACK: force SCR detection on TCH when SCR changes
    if (phys_ch->scr != PHYS_CH_SCR_DETECT) {
        phys_ch->scr = PHYS_CH_SCR_DETECT;
        phys_ch->scr_stat[scr] += 3;
    }

    return 0;
}
