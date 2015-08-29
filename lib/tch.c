#define LOG_PREFIX "tch"
#include <tetrapol/tch.h>
#include <stdlib.h>

struct tch_priv_t {
};

tch_t *tch_create(void)
{
    tch_t *tch = malloc(sizeof(tch_t));
    if (!tch) {
        return NULL;
    }

    return tch;
}

void tch_destroy(tch_t *tch)
{
    free(tch);
}

int tch_push_data_block(tch_t *tch, data_block_t *data_blk)
{
    return -1;
}

void tch_tick(const timeval_t *tv, void *tch)
{
}
