#include "bof3/bof3.h"

typedef void (*BmagicMagic05403Handler)(void);

typedef struct BmagicMagic05403DispatchTable {
  BmagicMagic05403Handler handlers[4];
} BmagicMagic05403DispatchTable;

/*
 * Code pointer run at 0x801EEC04, the table this dispatcher copies: entry 0 =
 * 0x801EEC98, entry 1 = 0x801EF05C, entry 2 = 0x801EF0B8, entry 3 = 0x801EF0EC.
 * The run continues past these four words with the six entries the sibling
 * dispatcher at 0x801EF19C copies.
 */
extern BmagicMagic05403DispatchTable dispatchHandlerTable801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC2C
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 selects an entry of the four-entry code pointer
 * table at 0x801EEC04 (0x801EEC98, 0x801EF05C, 0x801EF0B8, 0x801EF0EC), and
 * that entry is invoked with no arguments. The four table words are copied into
 * this function's own 0x28-byte frame first, so the call target is read back
 * from that copy. It reads no other state and writes none.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic05403_801EEC2C(void) {
  BmagicMagic05403DispatchTable handlers;

  handlers = dispatchHandlerTable801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
