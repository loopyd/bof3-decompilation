#include "bof3/bof3.h"

typedef void (*BmagicMagic00800Handler)(void);

typedef struct BmagicMagic00800DispatchTable {
  BmagicMagic00800Handler handlers[3];
} BmagicMagic00800DispatchTable;

/*
 * Code pointer table at 0x801EEC14, the table this dispatcher copies:
 * entry 0 = 0x801EF0B8, entry 1 = 0x801EF324, entry 2 = 0x801EF010.
 */
extern BmagicMagic00800DispatchTable D_801EEC14; /* @source 0x801EEC14 @kind data */

/* @source 0x801EF058
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 selects an entry of the three-entry code pointer
 * table at 0x801EEC14 (0x801EF0B8, 0x801EF324, 0x801EF010), and that entry is
 * invoked with no arguments. The three table words are copied into this
 * function's own 0x28-byte frame first, so the call target is read back from
 * the copy. It reads no other state and writes none.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic00800_801EF058(void) {
  BmagicMagic00800DispatchTable handlers;

  handlers = D_801EEC14;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
