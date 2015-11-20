/**
  This application create TETRAPOL channel bit strem for radio transmission.
  Input format is the same as used for tetrapol_dump.

  Output stream contains frames as packed bits (20B per frame).
 */
#include <tetrapol/frame.h>
#include <tetrapol/tetrapol.h>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int main_loop(FILE *in, FILE *out, frame_encoder_t *fe)
{
    return -1;
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

