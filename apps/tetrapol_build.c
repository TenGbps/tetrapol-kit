/**
  This application create TETRAPOL channel bit strem for radio transmission.
  Input format is the same as used for tetrapol_dump.

  Output stream contains frames as packed bits (20B per frame).
 */
#include <tetrapol/frame.h>
#include <tetrapol/tetrapol.h>

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

static int process_data_frame(FILE *out, json_object *json_frame,
        frame_encoder_t *fe, int line_no)
{
    uint8_t frame[20];

    // TODO: buld data frame
    memset(frame, 0, 20);

    return write_frame(frame, out);
}

static int process_voice_frame(FILE *out, json_object *json_frame,
        frame_encoder_t *fe, int line_no)
{
    uint8_t frame[20];

    // TODO: buld voice frame
    memset(frame, 0, 20);

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

