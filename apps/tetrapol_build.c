/**
  This application create TETRAPOL channel bit strem for radio transmission.
  Input format is the same as used for tetrapol_dump.

  Output stream contains frames as packed bits (20B per frame).
 */
#include <tetrapol/tetrapol.h>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    return -1;
}
