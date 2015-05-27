#include <tetrapol/terminal.h>
#include <stdlib.h>

#include <glib.h>

struct _terminal_t {
};

struct _terminal_list_t {
};

static terminal_t* terminal_create(void)
{
    return NULL;
}

static void terminal_destroy(terminal_t *term)
{
    free(term);
}

int terminal_push_hdlc_frame(terminal_t* term, const hdlc_frame_t *hdlc_fr,
        tsdu_t **tsdu)
{
    if (!tsdu) {
        return -1;
    }
    *tsdu = NULL;
    // TODO
    return -1;
}

terminal_list_t *terminal_list_create(void)
{
    return NULL;
}

void terminal_list_destroy(terminal_list_t *tlist)
{
    free(tlist);
}

terminal_t* terminal_list_lookup(const terminal_list_t* tlist, const addr_t *addr)
{
    return NULL;
}

terminal_t* terminal_list_insert(terminal_list_t* tlist, const addr_t *addr)
{
    return NULL;
}

void terminal_list_erase(terminal_list_t* tlist, const addr_t *addr)
{
    // TODO
}

int terminal_list_push_hdlc_frame(terminal_list_t* tlist,
        const hdlc_frame_t *hdlc_fr, tsdu_t **tsdu)
{
    if (!tsdu) {
        return -1;
    }
    *tsdu = NULL;
    // TODO
    return -1;
}

