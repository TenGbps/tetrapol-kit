#pragma once

#include <tetrapol/addr.h>
#include <tetrapol/hdlc_frame.h>
#include <tetrapol/tp_timer.h>
#include <tetrapol/tetrapol_int.h>

typedef struct terminal_priv_t terminal_t;

/**
  Push HDLC frame into terminal.

  @return 0 on sucess, -1 otherwise.
  */
int terminal_push_hdlc_frame(terminal_t* term, const hdlc_frame_t *hdlc_fr);

typedef struct terminal_list_priv_t terminal_list_t;

terminal_list_t *terminal_list_create(tpol_t *tpol, int log_ch);
void terminal_list_destroy(terminal_list_t *tlist);

/**
  Lookup terminal with specified address.

  @return pointer to terminal or NULL if not found.
  */
terminal_t* terminal_list_lookup(const terminal_list_t* tlist,
        const addr_t *addr);

/**
  Create new terminal structure and put it into list of terminals.

  @return Pointer to new terminal structure or NULL if error occures. If
  terminal with given address already exists just returns its address.
  */
terminal_t* terminal_list_insert(terminal_list_t* tlist, const addr_t *addr);

/**
  Remove terminal with given address from list.
  */
void terminal_list_erase(terminal_list_t* tlist, const addr_t *addr);

/**
  Push HDLC frame into terminal.

  @return 0 on sucess, -1 otherwise.
  */
int terminal_list_push_hdlc_frame(terminal_list_t* tlist,
        const hdlc_frame_t *hdlc_fr);

/**
  Report RX glitch to all terminals.
  */
void terminal_list_rx_glitch(terminal_list_t* tlist);

/**
  Call periodicaly to expire old sessions.
  @param tlist list with all terminals.
  @param te passing time event.
  */
void terminal_list_tick(terminal_list_t* tlist, time_evt_t *te);

