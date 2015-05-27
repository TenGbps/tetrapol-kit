#pragma once

#include <tetrapol/addr.h>
#include <tetrapol/hdlc_frame.h>
#include <tetrapol/timer.h>
#include <tetrapol/tsdu.h>

typedef struct _terminal_t terminal_t;

/**
  Push HDLC frame into terminal. If TSDU is compleded pointer to tsdu is set
  to new TSDU structure, otherwise is set to NULL.

  @return 0 on sucess, -1 otherwise.
  */
int terminal_push_hdlc_frame(terminal_t* term, const hdlc_frame_t *hdlc_fr,
        tsdu_t **tsdu);

typedef struct _terminal_list_t terminal_list_t;

terminal_list_t *terminal_list_create(void);
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
  Push HDLC frame into terminal. If TSDU is compleded pointer to tsdu is set
  to new TSDU structure, otherwise is set to NULL.

  @return 0 on sucess, -1 otherwise.
  */
int terminal_list_push_hdlc_frame(terminal_list_t* tlist,
        const hdlc_frame_t *hdlc_fr, tsdu_t **tsdu);

/**
  Call periodicaly to expire old sessions.
  @param tlist list with all terminals.
  @param tv current timestamp.
  */
void terminal_list_tick(terminal_list_t* tlist, const timeval_t *tv);

