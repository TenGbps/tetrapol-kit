#define LOG_PREFIX "terminal"

#include <tetrapol/log.h>
#include <tetrapol/terminal.h>
#include <tetrapol/link.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

struct terminal_priv_t {
    link_t *link;
};

struct terminal_list_priv_t {
    GTree *tree;
    tpol_t *tpol;
    int log_ch;
};

static terminal_t* terminal_create(tpol_t *tpol, int log_ch)
{
    terminal_t *term = malloc(sizeof(terminal_t));
    if (!term) {
        return NULL;
    }

    term->link = link_create(tpol, log_ch);
    if (!term->link) {
        free(term);
        return NULL;
    }

    return term;
}

static void terminal_destroy(terminal_t *term)
{
    if (!term) {
        return;
    }

    link_destroy(term->link);
    free(term);
}

static inline void terminal_destroy_(gpointer term)
{
    terminal_destroy(term);
}

int terminal_push_hdlc_frame(terminal_t* term, const hdlc_frame_t *hdlc_fr)
{
    return link_push_hdlc_frame(term->link, hdlc_fr);
}

static inline void addr_free(gpointer data)
{
    free(data);
}

static gint addr_cmp(gconstpointer _a1, gconstpointer _a2, gpointer user_data)
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

terminal_list_t *terminal_list_create(tpol_t * tpol, int log_ch)
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

    tlist->tpol = tpol;
    tlist->log_ch = log_ch;

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
    terminal_t *term = terminal_create(tlist->tpol, tlist->log_ch);
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
        const hdlc_frame_t *hdlc_fr)
{
    terminal_t *term = terminal_list_lookup(tlist, &hdlc_fr->addr);
    if (!term) {
        term = terminal_list_insert(tlist, &hdlc_fr->addr);
        if (!term) {
            LOG(ERR, "Terminal allocation failed");
            return -1;
        }
    }

    return terminal_push_hdlc_frame(term, hdlc_fr);
}

static gboolean terminal_rx_glitch(gpointer key, gpointer value,
        gpointer data)
{
    terminal_t *term = value;

    link_rx_glitch(term->link);

    return false;
}

void terminal_list_rx_glitch(terminal_list_t* tlist)
{
    g_tree_foreach(tlist->tree, terminal_rx_glitch, NULL);
}

static gboolean terminal_tick(gpointer key, gpointer value,
        gpointer data)
{
    terminal_t *term = value;
    time_evt_t *te = data;

    link_tick(te, term->link);

    return false;
}

void terminal_list_tick(terminal_list_t* tlist, time_evt_t *te)
{
    g_tree_foreach(tlist->tree, terminal_tick, te);
}

