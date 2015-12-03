#include <tetrapol/tetrapol.h>
// TODO: should use only tetrapol.h, but hi-level interface not implemented yet
#include <tetrapol/phys_ch.h>

#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// set on SIGINT
volatile static int do_exit = 0;


static void sigint_handler(int sig)
{
    do_exit = 1;
}

static int do_read(int fd, uint8_t *buf, int len)
{
    struct pollfd fds;
    fds.fd = fd;
    fds.events = POLLIN;
    fds.revents = 0;

    if (poll(&fds, 1, -1) > 0 && !do_exit) {
        if (! (fds.revents & POLLIN)) {
            return -1;
        }

        return read(fd, buf, len);
    }

    return do_exit ? 0 : -1;
}

static int tetrapol_dump_loop(phys_ch_t *phys_ch, int fd)
{
    int ret = 0;
    int data_len = 0;
    uint8_t data[4096];

    if (fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(fd, F_GETFL))) {
        return -1;
    }

    signal(SIGINT, sigint_handler);

    while (ret == 0 && !do_exit) {
        if (sizeof(data) - data_len > 0) {
            const int rsize = do_read(fd, data + data_len, sizeof(data) - data_len);
            if (rsize < 0) {
                return rsize;
            }
            if (!rsize && !data_len) {
                return 0;
            }
            data_len += rsize;
        }

        const int rsize = tetrapol_phys_ch_recv(phys_ch, data, data_len);
        if (rsize < 0) {
            return rsize;
        }
        if (rsize > 0) {
            memmove(data, data + rsize, data_len - rsize);
            data_len -= rsize;
        }

        ret = tetrapol_phys_ch_process(phys_ch);
    }

    return ret;
}

static void print_help(const char *prg_name)
{
    fprintf(stderr, "Decode data from demodulated TETRAPOL channel.\n");
    fprintf(stderr, "Usage: %s [OPTIONS ...]\n", prg_name);
    fprintf(stderr, "    -i <PATH>               input file with demodulated bits\n");
    fprintf(stderr, "    -b { UHF | VHF }        radio band (default is UHF\n");
    fprintf(stderr, "    -t { CCH | TCH }        select betwen control and traffic channel\n");
    fprintf(stderr, "    -d { DOWN | UP }        direction, downlink/direct or uplink\n");
}

int main(int argc, char* argv[])
{
    tetrapol_cfg_t cfg = {
        .band = TETRAPOL_BAND_UHF,
        .dir = DIR_DOWNLINK,
        .radio_ch_type = TETRAPOL_RADIO_CCH,
    };

    const char *in = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "b:hi:t:d:")) != -1) {
        switch (opt) {
            case 'b':
                if (!strcmp(optarg, "VHF")) {
                    cfg.band = TETRAPOL_BAND_VHF;
                } else if (!strcmp(optarg, "UHF")) {
                    cfg.band = TETRAPOL_BAND_UHF;
                } else {
                    print_help(argv[0]);
                    exit(EXIT_FAILURE);
                }
                break;

            case 'i':
                in = optarg;
                break;

            case 't':
                if (!strcmp("CCH", optarg)) {
                    cfg.radio_ch_type = TETRAPOL_RADIO_CCH;
                } else if (!strcmp("TCH", optarg)) {
                    cfg.radio_ch_type = TETRAPOL_RADIO_TCH;
                } else {
                    print_help(argv[0]);
                    exit(EXIT_FAILURE);
                }
                break;

            case 'h':
                print_help(argv[0]);
                exit(0);
                break;

            case 'd':
                if (!strcmp("UP", optarg)) {
                    cfg.dir = DIR_UPLINK;
                } else if (!strcmp("DOWN", optarg)) {
                    cfg.dir = DIR_DOWNLINK;
                } else {
                    print_help(argv[0]);
                    exit(EXIT_FAILURE);
                }
                break;

            default:
                print_help(argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    int infd = STDIN_FILENO;
    if (in && strcmp(in, "-")) {
        infd = open(in, O_RDONLY);
        if (infd == -1) {
            perror("Failed to open input file");
            return -1;
        }
    }

    tetrapol_t *tetrapol = tetrapol_create(&cfg);
    if (tetrapol == NULL) {
        fprintf(stderr, "Failed to initialize TETRAPOL instance.");
        return -1;
    }
    phys_ch_t *phys_ch = tetrapol_phys_ch_create(tetrapol);
    if (phys_ch == NULL) {
        fprintf(stderr, "Failed to initialize TETRAPOL instance.");
        return -1;
    }

    const int ret = tetrapol_dump_loop(phys_ch, infd);
    tetrapol_phys_ch_destroy(phys_ch);
    if (infd != STDIN_FILENO) {
        close(infd);
    }
    tetrapol_destroy(tetrapol);

    fprintf(stderr, "Exiting.\n");

    return ret;
}
