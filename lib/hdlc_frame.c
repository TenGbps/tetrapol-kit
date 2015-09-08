#define LOG_PREFIX "hdlc"
#include <tetrapol/log.h>
#include <tetrapol/hdlc_frame.h>
#include <tetrapol/bit_utils.h>
#include <tetrapol/misc.h>

#include <stdbool.h>
#include <string.h>

static const command_mask_t commands[] = {
    {   .cmd = COMMAND_INFORMATION,         .mask = 0x01 },
    {   .cmd = COMMAND_SUPERVISION_RR,      .mask = 0x0f },
    {   .cmd = COMMAND_SUPERVISION_RNR,     .mask = 0x0f },
    {   .cmd = COMMAND_SUPERVISION_REJ,     .mask = 0x0f },
    {   .cmd = COMMAND_DACH,                .mask = 0x0f },
    {   .cmd = COMMAND_UNNUMBERED_UI,       .mask = 0xef },
    {   .cmd = COMMAND_UNNUMBERED__BLANK1,  .mask = 0xef },
    {   .cmd = COMMAND_UNNUMBERED_DISC,     .mask = 0xef },
    {   .cmd = COMMAND_UNNUMBERED_UA,       .mask = 0xef },
    {   .cmd = COMMAND_UNNUMBERED_SNRM,     .mask = 0xef },
    {   .cmd = COMMAND_UNNUMBERED_UI_CD,    .mask = 0xff },
    {   .cmd = COMMAND_UNNUMBERED_UI_VCH,   .mask = 0xff },
    {   .cmd = COMMAND_UNNUMBERED__BLANK2,  .mask = 0xef },
    {   .cmd = COMMAND_UNNUMBERED_UI_P0,    .mask = 0xef },
    {   .cmd = COMMAND_UNNUMBERED_U_RR,     .mask = 0x2f },
    {   .cmd = COMMAND_UNNUMBERED_FRMR,     .mask = 0xef },
    {   .cmd = COMMAND_UNNUMBERED__BLANK3,  .mask = 0x0f },
    {   .cmd = COMMAND_UNNUMBERED_DM,       .mask = 0xef },
    //{   .cmd = COMMAND_UNNUMBERED__BLANK4,  .mask = 0xef },
};

static void command_parse(command_t *cmd, uint8_t data)
{
    cmd->cmd = data;

    int cmd_idx = 0;
    for ( ; cmd_idx < ARRAY_LEN(commands); ++cmd_idx) {
        if ((data & commands[cmd_idx].mask) == commands[cmd_idx].cmd) {
            cmd->cmd = commands[cmd_idx].cmd;
            break;
        }
    }

    switch (cmd->cmd) {
        case COMMAND_INFORMATION:
            cmd->information.n_r            = get_bits(3, &data, 0);
            cmd->information.p_e            = get_bits(1, &data, 3);
            cmd->information.n_s            = get_bits(3, &data, 4);
            break;

        case COMMAND_SUPERVISION_RR:
        case COMMAND_SUPERVISION_RNR:
        case COMMAND_SUPERVISION_REJ:
            cmd->supervision.n_r            = get_bits(3, &data, 0);
            cmd->supervision.p_e            = get_bits(1, &data, 3);
            break;

        case COMMAND_DACH:
            cmd->dach_access.seq_no         = get_bits(3, &data, 0);
            cmd->dach_access.retry          = get_bits(1, &data, 3);
            break;

        case COMMAND_UNNUMBERED_UI:
        case COMMAND_UNNUMBERED_DISC:
        case COMMAND_UNNUMBERED_UA:
        case COMMAND_UNNUMBERED_SNRM:
            cmd->unnumbered.p_e             = get_bits(1, &data, 3);
            break;

        case COMMAND_UNNUMBERED_UI_CD:
        case COMMAND_UNNUMBERED_UI_VCH:
            break;

        case COMMAND_UNNUMBERED_UI_P0:
            cmd->unnumbered.ra              = get_bits(1, &data, 3);
            break;

        case COMMAND_UNNUMBERED_U_RR:
            cmd->unnumbered.response_format = get_bits(2, &data, 0);
            cmd->unnumbered.p_e             = get_bits(1, &data, 3);
            break;

        case COMMAND_UNNUMBERED_FRMR:
        case COMMAND_UNNUMBERED_DM:
            cmd->unnumbered.p_e             = get_bits(1, &data, 3);
            break;

        case COMMAND_UNNUMBERED__BLANK1:
        case COMMAND_UNNUMBERED__BLANK2:
        case COMMAND_UNNUMBERED__BLANK3:
        default:
            LOG(WTF, "Unknown command 0x%02x", data);
    };
}

bool hdlc_frame_parse(hdlc_frame_t *hdlc_frame, const uint8_t *data, int nbits)
{
    addr_parse(&hdlc_frame->addr, data, 0);
    command_parse(&hdlc_frame->command, data[2]);
    // nbits - HDLC_header_nbits - FCS_nbits
    hdlc_frame->nbits = nbits - 3*8 - 2*8;
    // copy FCS behind the data for future use
    memcpy(hdlc_frame->data, data + 3, (hdlc_frame->nbits + 2*8 + 7) / 8);

    return check_fcs(data, nbits);
}

/**
  PAS 0001-3-3 7.4.1.9

  Python 3 script to generate stuffing pattern list.

seq = [0, 0, 0, 0, 1]

for i in range(5, 40):
    seq.append(seq[i-3] ^ seq[i-5])
seq.reverse()

res = []
for i in range(40):
    data = eval('0b' + ''.join([str(b) for b in seq]))
    res.append("""    { .data = { 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, }, .index = %d, },""" % (
        (data >> 32) & 0xff,
        (data >> 24) & 0xff,
        (data >> 16) & 0xff,
        (data >> 8) & 0xff,
        (data >> 0) & 0xff,
        i)
    )
    seq =  seq[1:] + seq[:1]
res.sort()
for l in res:
    print(l)
  */
static const struct {
    uint8_t data[5];
    uint8_t index;
} stuff_pat[40] = {
    { .data = { 0x04, 0x85, 0x76, 0x3e, 0x69, }, .index = 36, },
    { .data = { 0x09, 0x0a, 0xec, 0x7c, 0xd2, }, .index = 37, },
    { .data = { 0x0a, 0xec, 0x7c, 0xd2, 0x09, }, .index = 5, },
    { .data = { 0x12, 0x15, 0xd8, 0xf9, 0xa4, }, .index = 38, },
    { .data = { 0x15, 0xd8, 0xf9, 0xa4, 0x12, }, .index = 6, },
    { .data = { 0x1f, 0x34, 0x82, 0x42, 0xbb, }, .index = 19, },
    { .data = { 0x20, 0x90, 0xae, 0xc7, 0xcd, }, .index = 33, },
    { .data = { 0x21, 0x5d, 0x8f, 0x9a, 0x41, }, .index = 2, },
    { .data = { 0x24, 0x2b, 0xb1, 0xf3, 0x48, }, .index = 39, },
    { .data = { 0x2b, 0xb1, 0xf3, 0x48, 0x24, }, .index = 7, },
    { .data = { 0x34, 0x82, 0x42, 0xbb, 0x1f, }, .index = 27, },
    { .data = { 0x3e, 0x69, 0x04, 0x85, 0x76, }, .index = 20, },
    { .data = { 0x41, 0x21, 0x5d, 0x8f, 0x9a, }, .index = 34, },
    { .data = { 0x42, 0xbb, 0x1f, 0x34, 0x82, }, .index = 3, },
    { .data = { 0x48, 0x24, 0x2b, 0xb1, 0xf3, }, .index = 31, },
    { .data = { 0x48, 0x57, 0x63, 0xe6, 0x90, }, .index = 0, },
    { .data = { 0x57, 0x63, 0xe6, 0x90, 0x48, }, .index = 8, },
    { .data = { 0x5d, 0x8f, 0x9a, 0x41, 0x21, }, .index = 10, },
    { .data = { 0x63, 0xe6, 0x90, 0x48, 0x57, }, .index = 16, },
    { .data = { 0x69, 0x04, 0x85, 0x76, 0x3e, }, .index = 28, },
    { .data = { 0x76, 0x3e, 0x69, 0x04, 0x85, }, .index = 12, },
    { .data = { 0x7c, 0xd2, 0x09, 0x0a, 0xec, }, .index = 21, },
    { .data = { 0x82, 0x42, 0xbb, 0x1f, 0x34, }, .index = 35, },
    { .data = { 0x85, 0x76, 0x3e, 0x69, 0x04, }, .index = 4, },
    { .data = { 0x8f, 0x9a, 0x41, 0x21, 0x5d, }, .index = 18, },
    { .data = { 0x90, 0x48, 0x57, 0x63, 0xe6, }, .index = 32, },
    { .data = { 0x90, 0xae, 0xc7, 0xcd, 0x20, }, .index = 1, },
    { .data = { 0x9a, 0x41, 0x21, 0x5d, 0x8f, }, .index = 26, },
    { .data = { 0xa4, 0x12, 0x15, 0xd8, 0xf9, }, .index = 30, },
    { .data = { 0xae, 0xc7, 0xcd, 0x20, 0x90, }, .index = 9, },
    { .data = { 0xb1, 0xf3, 0x48, 0x24, 0x2b, }, .index = 15, },
    { .data = { 0xbb, 0x1f, 0x34, 0x82, 0x42, }, .index = 11, },
    { .data = { 0xc7, 0xcd, 0x20, 0x90, 0xae, }, .index = 17, },
    { .data = { 0xcd, 0x20, 0x90, 0xae, 0xc7, }, .index = 25, },
    { .data = { 0xd2, 0x09, 0x0a, 0xec, 0x7c, }, .index = 29, },
    { .data = { 0xd8, 0xf9, 0xa4, 0x12, 0x15, }, .index = 14, },
    { .data = { 0xe6, 0x90, 0x48, 0x57, 0x63, }, .index = 24, },
    { .data = { 0xec, 0x7c, 0xd2, 0x09, 0x0a, }, .index = 13, },
    { .data = { 0xf3, 0x48, 0x24, 0x2b, 0xb1, }, .index = 23, },
    { .data = { 0xf9, 0xa4, 0x12, 0x15, 0xd8, }, .index = 22, },
};

int hdlc_frame_stuffing_idx(const hdlc_frame_t *hdlc_frame)
{
    if (!addr_is_tti_no_st(&hdlc_frame->addr, true) ||
            hdlc_frame->command.cmd != COMMAND_UNNUMBERED_UI ||
            (hdlc_frame->nbits + 2*8) != 5*8) {
        return -1;
    }

    int pos = ARRAY_LEN(stuff_pat) / 2;
    int lo = 0, hi = ARRAY_LEN(stuff_pat) - 1;
    while (lo < hi) {
        const int cmp = memcmp(hdlc_frame->data, stuff_pat[pos].data, 5);
        if (!cmp) {
            return stuff_pat[pos].index;
        }
        if (pos == lo) {
            ++pos;
        } else {
            if (cmp > 0) {
                lo = pos + 1;
            } else {
                hi = pos - 1;
            }
            pos = (lo + hi) / 2;
        }
    }

    return memcmp(hdlc_frame->data, stuff_pat[pos].data, 5) ?
        -1 : stuff_pat[pos].index;
}

