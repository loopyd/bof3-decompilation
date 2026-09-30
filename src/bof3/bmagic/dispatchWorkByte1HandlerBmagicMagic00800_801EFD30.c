#include "bof3/bof3.h"

typedef void (*BmagicMagic00800Handler)(void);

typedef struct BmagicMagic00800DispatchTable {
  BmagicMagic00800Handler handlers[3];
} BmagicMagic00800DispatchTable;

/*
 * Code pointer table at 0x801EEC4C, the table this dispatcher copies:
 * entry 0 = 0x801EFD90, entry 1 = 0x801F0018, entry 2 = 0x801F01E4.
 */
extern BmagicMagic00800DispatchTable D_801EEC4C; /* @source 0x801EEC4C @kind data */

/* @source 0x801EFD30
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 selects an entry of the three-entry code pointer
 * table at 0x801EEC4C (0x801EFD90, 0x801F0018, 0x801F01E4), and that entry is
 * invoked with no arguments. As at 0x801EF058 and 0x801EFA74, the three table
 * words are copied into this function's own 0x28-byte frame first, so the call
 * target is read back from the copy. It reads no other state and writes none.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic00800_801EFD30(void) {
  BmagicMagic00800DispatchTable handlers;

  handlers = D_801EEC4C;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
