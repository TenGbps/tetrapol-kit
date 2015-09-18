#include <tetrapol/addr.h>
#include <stdio.h>

char* addr_print(char *buf, const addr_t *addr)
{
    sprintf(buf, "ADDR=%d.%d.0x%03x", addr->z, addr->y, addr->x);

    return buf;
}
