#include "bof3/bof3.h"

typedef void (*BmagicMagic05703Handler)(void);

typedef struct BmagicMagic05703DispatchTable {
  BmagicMagic05703Handler handlers[3];
} BmagicMagic05703DispatchTable;

/*
 * Code pointer table at 0x801EEC20, the table this dispatcher copies:
 * entry 0 = 0x801EF08C, entry 1 = 0x801EF55C, entry 2 = 0x801EF83C.
 */
extern BmagicMagic05703DispatchTable D_801EEC20; /* @source 0x801EEC20 @kind data */

/* @source 0x801EF02C
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 selects an entry of the three-entry code pointer
 * table at 0x801EEC20 (0x801EF08C, 0x801EF55C, 0x801EF83C), and that entry is
 * invoked with no arguments. The three table words are copied into this
 * function's own 0x28-byte frame first, so the call target is read back from
 * the copy. The index is not bounds-checked. It reads no other state and
 * writes none.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic05703_801EF02C(void) {
  BmagicMagic05703DispatchTable handlers;

  handlers = D_801EEC20;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
