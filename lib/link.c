#define LOG_PREFIX "link"

#include <tetrapol/link.h>
#include <tetrapol/log.h>

#include <stdlib.h>

struct link_priv_t {
    int x;
};

link_t *link_create(void)
{
    return malloc(sizeof(link_t));
}

void link_destroy(link_t *link)
{
    free(link);
}

int link_push_hdlc_frame(link_t *link, const hdlc_frame_t *hdlc_fr, tsdu_t **tsdu)
{
    return -1;
}

void link_tick(const timeval_t* tv, link_t *link)
{
}
