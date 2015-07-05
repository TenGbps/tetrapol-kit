#include <tetrapol/log.h>
#include <tetrapol/misc.h>

void print_hex(const uint8_t *bytes, int n)
{
    for(int i = 0; i < n; i++) {
        LOGF("%02x ", bytes[i]);
        if (i % 8 == 7) {
            LOGF(" ");
        }
    }
    LOGF("\n");
}
