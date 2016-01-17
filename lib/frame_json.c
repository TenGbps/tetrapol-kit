#include <tetrapol/frame_json.h>
#include <tetrapol/misc.h>

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

void frame_json(tpol_t *tpol, const frame_t *fr)
{
    printf("{ \"event\": \"frame\", ");
    printf("\"rx_offs\": %" PRIu64 ", ", tpol->rx_offs);

    struct timeval tv;
    struct tm gmt;
    gettimeofday(&tv, NULL);
    gmtime_r(&tv.tv_sec, &gmt);

    printf("\"rx_time\": \"%4d-%02d-%02dT%02d-%02d-%02d.%06ld\", ",
            gmt.tm_year + 1900, gmt.tm_mon + 1, gmt.tm_mday,
            gmt.tm_hour, gmt.tm_min, gmt.tm_sec, tv.tv_usec);


    printf("\"frame\": { ");
    {
        if (tpol->frame_no != FRAME_NO_UNKNOWN) {
            printf("\"frame_no\": %d, ", tpol->frame_no);
        } else {
            printf("\"frame_no\": null, ");
        }

        if (!fr->broken) {
            printf("\"state\": \"ok\", ");
            printf("\"syndromes\": %d, ", fr->syndromes);
            printf("\"bits_fixed\": %d, ", fr->bits_fixed);

            const char *fr_type;
            switch (fr->fr_type) {
                case FRAME_TYPE_VOICE:
                    fr_type = "VOICE";
                    break;

                case FRAME_TYPE_DATA:
                    fr_type = "DATA";
                    break;

                default:
                    fr_type = "FIXME";
            }
            printf("\"type\": \"%s\", ", fr_type);

            if (fr->fr_type == FRAME_TYPE_DATA) {
                printf("\"asb\": [%d, %d], ", fr->data.asb[0], fr->data.asb[1]);
                printf("\"fn\": [%d, %d], ", fr->data.data[0], fr->data.data[1]);

                uint8_t data[8];
                memset(data, 0, sizeof(data));
                for (int i = 0; i < 8*8; ++i) {
                    data[i / 8] |= fr->data.data[i + 2] << (i % 8);
                }
                char buf[3*sizeof(data)];
                printf("\"data\": { \"encoding\": \"hex\", \"value\": \"%s\" } ",
                        sprint_hex2(buf, data, sizeof(data)));

            } else if (fr->fr_type == FRAME_TYPE_VOICE) {
                printf("\"asb\": [%d, %d], ", fr->voice.asb[0], fr->voice.asb[1]);
                uint8_t voice[120/8];
                memset(voice, 0, sizeof(voice));

                for (int i = 0; i < 20; ++i) {
                    voice[i / 8] |= fr->voice.voice1[i] << (i % 8);
                }
                for (int i = 20; i < 120; ++i) {
                    voice[i / 8] |= fr->voice.voice2[i - 20] << (i % 8);
                }

                char buf[120/8*3];
                printf("\"data\": { \"encoding\": \"hex\", \"value\": \"%s\" } ",
                        sprint_hex2(buf, voice, 120/8));

            } else {
                printf("\"FIXME\": \"FIXME\" ");
            }
        } else if (fr->broken == -1) {
            printf("\"state\": \"bad_CRC\", ");
            printf("\"syndromes\": %d, ", fr->syndromes);
            printf("\"bits_fixed\": %d ", fr->bits_fixed);
        } else if (fr->broken > 0) {
            printf("\"state\": %d, ", fr->broken);
        } else {
            printf("\"state\": \"FIXME\", ");
        }
    }
    printf("}");

    printf("}\n");
}
