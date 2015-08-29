#define LOG_PREFIX "cch"
#include <tetrapol/cch.h>
#include <tetrapol/log.h>
#include <stdlib.h>

struct cch_priv_t {
};

cch_t *cch_create(void)
{
    cch_t *cch = malloc(sizeof(cch_t));
    if (!cch) {
        return NULL;
    }

    return cch;
}

void cch_destroy(cch_t *cch)
{
    free(cch);
}

int cch_push_data_block(cch_t *cch, data_block_t *data_blk)
{
    return -1;
}

void cch_fr_error(cch_t *cch)
{
}

void cch_tick(const timeval_t *tv, void *cch)
{
}
