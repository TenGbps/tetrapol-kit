#include <tetrapol/terminal.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

struct _terminal_t {
};

struct _terminal_list_t {
    GTree *tree;
};

static terminal_t* terminal_create(void)
{
    terminal_t *term = malloc(sizeof(terminal_t));
    if (!term) {
        return NULL;
    }
    memset(term, 0, sizeof(terminal_t));

    return term;
}

static void terminal_destroy(terminal_t *term)
{
    free(term);
}

static inline void terminal_destroy_(gpointer term)
{
    terminal_destroy(term);
}

int terminal_push_hdlc_frame(terminal_t* term, const hdlc_frame_t *hdlc_fr,
        tsdu_t **tsdu)
{
    if (!tsdu) {
        LOG(ERR, "terminal_push_hdlc_frame() tsdu == NULL");
        return -1;
    }
    *tsdu = NULL;
    // TODO
    return -1;
}

static inline void addr_free(gpointer data)
{
    free(data);
}

gint addr_cmp(gconstpointer _a1, gconstpointer _a2, gpointer user_data)
{
    const addr_t *a1 = _a1;
    const addr_t *a2 = _a2;

    if (a1->z != a2->z) {
        return a2->z - a1->z;
    }
    if (a1->y != a2->y) {
        return a2->y - a1->y;
    }
    return a2->x - a1->x;
}

terminal_list_t *terminal_list_create(void)
{
    terminal_list_t *tlist = malloc(sizeof(terminal_list_t));
    if (!tlist) {
        return NULL;
    }

    tlist->tree = g_tree_new_full(addr_cmp, NULL, addr_free, terminal_destroy_);
    if (!tlist->tree) {
        free(tlist);
        return NULL;
    }

    return tlist;
}

void terminal_list_destroy(terminal_list_t *tlist)
{
    g_tree_destroy(tlist->tree);
    free(tlist);
}

terminal_t* terminal_list_lookup(const terminal_list_t* tlist, const addr_t *addr)
{
    return g_tree_lookup(tlist->tree, addr);
}

terminal_t* terminal_list_insert(terminal_list_t* tlist, const addr_t *addr)
{
    terminal_t *term = terminal_create();
    if (!term) {
        return NULL;
    }

    addr_t *addr_new = malloc(sizeof(addr_t));
    if (!addr_new) {
        terminal_destroy(term);
        return NULL;
    }
    memcpy(addr_new, addr, sizeof(addr_t));

    g_tree_insert(tlist->tree, addr_new, term);

    return term;
}

void terminal_list_erase(terminal_list_t* tlist, const addr_t *addr)
{
    g_tree_remove(tlist->tree, addr);
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

