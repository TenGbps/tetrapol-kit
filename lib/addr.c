#include <tetrapol/addr.h>
#include <tetrapol/log.h>

void addr_print(const addr_t *addr)
{
    LOGF("ADDR=%d.%d.0x%03x", addr->z, addr->y, addr->x);
}
