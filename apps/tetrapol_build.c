/**
  This application create TETRAPOL channel bit strem for radio transmission.
  Input format is the same as used for tetrapol_dump.

  Output stream contains frames as packed bits (20B per frame).
 */
#include <tetrapol/frame.h>
#include <tetrapol/tetrapol.h>
#include <tetrapol/frame.h>

#include <json-c/json.h>
#include <errno.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PRINT_ERR(fmt, ...) \
    fprintf(stderr, "Error at line %d: " fmt "\n", __VA_ARGS__)

static int write_frame(const uint8_t *frame, FILE *out)
{
    uint8_t buf[160];

    for (uint8_t i = 0; i < 20; ++i) {
        uint8_t b = frame[i];
        for (uint8_t j = 0; j < 8; ++j) {
            buf[i*8 + j] = !!(b & 0x80);
            b <<= 1;
        }
    }

    return fwrite(buf, sizeof(buf), 1, out) != 1;
}

static int get_frame_data(uint8_t *data, int n, json_object *json_frame, int line_no)
{
    json_object *json_frame_data;

    if (!json_object_object_get_ex(json_frame, "data", &json_frame_data)) {
        PRINT_ERR("missing 'frame/data' keys", line_no);
        return -1;
    }

    json_object *json_data_encoding;
    if (!json_object_object_get_ex(json_frame_data, "encoding", &json_data_encoding)) {
        PRINT_ERR("faile to get 'frame/data/encoding' key", line_no);
        return -1;
    }

    const char *data_encoding = json_object_get_string(json_data_encoding);
    if (strcmp("hex", data_encoding)) {
        PRINT_ERR("unsupported data encoding: '%s'",line_no, data_encoding);
        return -1;
    }

    json_object *json_value;
    if (!json_object_object_get_ex(json_frame_data, "value", &json_value)) {
        PRINT_ERR("faile to get 'frame/data/value' key", line_no);
        return -1;
    }
    const char *value_str = json_object_get_string(json_value);

    if (strlen(value_str) != 2*n) {
        PRINT_ERR("invalid lenght of frame/data/value content", line_no);
        return -1;
    }

    // valudete value_str for hexadecimal
    if (strlen(value_str) != strspn(value_str, "0123456789abcdefABCDEF")) {
        PRINT_ERR("illegal data value", line_no);
        return -1;
    }

    char value[2*n+1];
    memcpy(value, value_str, 2*n+1);
    for (uint8_t i = n; i; ) {
        --i;
        data[i] = atoi(&value[2*i]);
        value[2*i] = 0;
    }

    return 0;
}

/// used to get value of ABS and FN
static int get_2bits(uint8_t *bits, json_object *json, int line_no)
{
    json_object *json_val = json_object_array_get_idx(json, 0);
    if (!json_val) {
        PRINT_ERR("faile to get json[0]", line_no);
        return -1;
    }
    bits[0] = json_object_get_int(json_val);
    if (errno != 0) {
        PRINT_ERR("invalid bits value", line_no);
        return -1;
    }

    json_val = json_object_array_get_idx(json, 1);
    if (!json_val) {
        PRINT_ERR("faile to get json[1]", line_no);
        return -1;
    }
    bits[1] = json_object_get_int(json_val);
    if (errno != 0) {
        PRINT_ERR("invalid bits value", line_no);
        return -1;
    }

    return 0;
}

static int get_asb(uint8_t *asb, json_object *json_frame, int line_no)
{
    json_object *json_asb;
    if (!json_object_object_get_ex(json_frame, "asb", &json_asb)) {
        PRINT_ERR("failed to get ASB filed", line_no);
        return -1;
    }

    return get_2bits(asb, json_asb, line_no);
}

static int get_fn(uint8_t *fn, json_object *json_frame, int line_no)
{
    json_object *json_fn;
    if (!json_object_object_get_ex(json_frame, "fn", &json_fn)) {
        PRINT_ERR("failed to get FN", line_no);
        return -1;
    }

    return get_2bits(fn, json_fn, line_no);
}

static int process_data_frame(FILE *out, json_object *json_frame,
        frame_encoder_t *fe, int line_no)
{
    frame_t fr;
    uint8_t frame[20];
    int r;

    fr.fr_type = FRAME_TYPE_DATA;

    uint8_t fr_data[8];
    r = get_frame_data(fr_data, sizeof(fr_data), json_frame, line_no);
    if (r) {
        return r;
    }
    for (uint8_t i = 0; i < 64; ++i) {
        fr.data.data[i+2] = (fr_data[i / 8] >> (i % 8)) & 0x01;
    }

    if (get_fn(fr.data.data, json_frame, line_no)) {
        return -1;
    }

    if (get_asb(fr.data.asb, json_frame, line_no)) {
        return -1;
    }

    if (frame_encoder_encode(fe, frame, &fr) == -1) {
        PRINT_ERR("data frame encoding failed", line_no);
        return -1;
    }

    return write_frame(frame, out);
}

static int process_voice_frame(FILE *out, json_object *json_frame,
        frame_encoder_t *fe, int line_no)
{
    uint8_t frame[20];
    frame_t fr;
    int r;

    fr.fr_type = FRAME_TYPE_VOICE;

    uint8_t fr_data[15];
    r = get_frame_data(fr_data, sizeof(fr_data), json_frame, line_no);
    if (r) {
        return r;
    }
    for (uint8_t i = 0; i < 20; ++i) {
        fr.voice.voice1[i] = (fr_data[i / 8] >> (i % 8)) & 0x01;
    }
    for (uint8_t i = 20; i < 120; ++i) {
        fr.voice.voice2[i - 20] = (fr_data[i / 8] >> (i % 8)) & 0x01;
    }

    if (get_asb(fr.voice.asb, json_frame, line_no)) {
        return -1;
    }

    if (frame_encoder_encode(fe, frame, &fr) == -1) {
        PRINT_ERR("voice frame encoding error at line", line_no);
        return -1;
    }

    return write_frame(frame, out);
}

/**
  Read lines from input.
  @return 0 when all lines were processes sucessfully, -1 on error.
  */
static int main_loop(FILE *in, FILE *out, frame_encoder_t *fe)
{
    char line[4001];
    int line_no = 0;
    int r;

    while ( (r = fscanf(in, "%4000[^\n]\n", line)) != EOF ) {
        if (r != 1) {
            PRINT_ERR("fscanf failes - %d != 1", line_no, r);
            return -1;
        }

        ++line_no;
        if (!strlen(line)) {
            continue;
        }
        if (line[0] == '#') {
            continue;
        }

        json_object *json = json_tokener_parse(line);

        json_object *json_event;
        if (!json_object_object_get_ex(json, "event", &json_event)) {
            PRINT_ERR("missing 'event' key", line_no);
            json_object_put(json);
            return -1;
        }

        const char *event = json_object_get_string(json_event);
        if (!strcmp("scr", event)) {
            json_object *json_scr;
            if (!json_object_object_get_ex(json, "scr", &json_scr)) {
                PRINT_ERR("missing 'scr' key", line_no);
                json_object_put(json);
                return -1;
            }
            int scr = json_object_get_int(json_scr);
            frame_encoder_set_scr(fe, scr);
            json_object_put(json);
            continue;
        }

        if (strcmp("frame", event)) {
            json_object_put(json);
            continue;
        }

        json_object *json_frame;
        if (!json_object_object_get_ex(json, "frame", &json_frame)) {
            PRINT_ERR("missing 'frame' key", line_no);
            json_object_put(json);
            return -1;
        }

        json_object *json_frame_type;
        if (!json_object_object_get_ex(json_frame, "type", &json_frame_type)) {
            PRINT_ERR("missing 'frame/type' keys", line_no);
            json_object_put(json);
            return -1;
        }
        const char *frame_type = json_object_get_string(json_frame_type);

        if (!strcmp("DATA", frame_type)) {
            r = process_data_frame(out, json_frame, fe, line_no);
            json_object_put(json);
            if (r) {
                return r;
            }
            continue;
        }

        if (!strcmp("VOICE", frame_type)) {
            r = process_voice_frame(out, json_frame, fe, line_no);
            json_object_put(json);
            if (r) {
                return r;
            }
            continue;
        }

        PRINT_ERR("unsupported frame type '%s'", line_no, frame_type);
        json_object_put(json);
    }

    if (errno != 0) {
        perror("Failed to read input file");
        return -1;
    }
    return 0;
}

static void print_help(const char *prg_name)
{
    fprintf(stderr,
            "Usage: %s [-b { UHF | VHF }] [-d { DOWN | UP }] [-i <INPUT_FILE>] [-o <OUTPUT_FILE>]\n",
            prg_name);
}

int main(int argc, char* argv[])
{
    const char *in = NULL;
    const char *out = NULL;
    int band = TETRAPOL_BAND_UHF;
    int dir = DIR_DOWNLINK;

    int opt;
    while ((opt = getopt(argc, argv, "b:d:hi:o:")) != -1) {
        switch (opt) {
            case 'b':
                if (!strcmp(optarg, "VHF")) {
                    band = TETRAPOL_BAND_VHF;
                } else if (!strcmp(optarg, "UHF")) {
                    band = TETRAPOL_BAND_UHF;
                } else {
                    fprintf(stderr, "Invalid band\n");
                    print_help(argv[0]);
                    return -1;
                }
                break;

            case 'd':
                if (!strcmp(optarg, "DOWN")) {
                    dir = DIR_DOWNLINK;
                } else if (!strcmp(optarg, "UP")) {
                    dir = DIR_UPLINK;
                } else {
                    fprintf(stderr, "Invalid direction\n");
                    print_help(argv[0]);
                    return -1;
                }
                break;

            case 'i':
                if (!strcmp(optarg, "-")) {
                    in = NULL;
                } else {
                    in = optarg;
                }
                break;

            case 'o':
                if (!strcmp(optarg, "-")) {
                    in = NULL;
                } else {
                    out = optarg;
                }
                break;

            case 'h':
                print_help(argv[0]);
                exit(0);
                break;

            default:
                print_help(argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    FILE *in_file;
    FILE *out_file;

    if (!in) {
        in_file = stdin;
    } else {
        in_file = fopen(in, "r");
        if (!in_file) {
            perror("Failed to open input file");
            print_help(argv[0]);
            return -1;
        }
    }

    if (!out) {
        out_file = stdout;
    } else {
        out_file = fopen(out, "w");
        if (!out_file) {
            if (in_file != stdin) {
                fclose(in_file);
            }
            perror("Failed to open output file");
            print_help(argv[0]);
            return -1;
        }
    }

    frame_encoder_t *fe = frame_encoder_create(band, 0, dir);
    int r = main_loop(in_file, out_file, fe);
    frame_encoder_destroy(fe);
    if (in_file != stdin) {
        fclose(in_file);
    }
    if (out_file != stdout) {
        fclose(out_file);
    }

    return r;
}

