#include <tetrapol/frame_json.h>
#include <tetrapol/misc.h>

#include <stdio.h>
#include <string.h>

void frame_json(tpol_t *tpol, const frame_t *fr)
{
    printf("{ \"event\": \"frame\", ");
    printf("\"rx_offs\": %lu, ", tpol->rx_offs);

    printf("\"frame\": { ");
    {
        if (tpol->frame_no != FRAME_NO_UNKNOWN) {
            printf("\"frame_no\": %d, ", tpol->frame_no);
        } else {
            printf("\"frame_no\": null, ");
        }

        if (!fr->errors) {

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
        } else {
            printf("\"FIXME\": \"FIXME\" ");
        }
    }
    printf("}");

    printf("}\n");
}
