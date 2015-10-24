#include <tetrapol/log.h>
#include <tetrapol/misc.h>
#include <stdio.h>

char *sprint_hex(char *str, const uint8_t *bytes, int n)
{
    for(int i = 0; i < n; ) {
        // TODO: replace sprintf with straight conversion
        sprintf(&str[3*i], "%02x ", bytes[i]);
        ++i;
        if (i == n) {
            break;
        }
    }
    if (n == 0) {
        str[0] = 0;
    } else {
        str[3*(n-1) + 2] = 0;
    }

    return str;
}

char *sprint_hex2(char *str, const uint8_t *bytes, int n)
{
    for(int i = 0; i < n; ) {
        // TODO: replace sprintf with straight conversion
        sprintf(&str[2*i], "%02x", bytes[i]);
        ++i;
        if (i == n) {
            break;
        }
    }
    str[2*n] = 0;

    return str;
}

