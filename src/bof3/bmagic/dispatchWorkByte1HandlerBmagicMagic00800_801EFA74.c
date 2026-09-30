#include "bof3/bof3.h"

typedef void (*BmagicMagic00800Handler)(void);

typedef struct BmagicMagic00800DispatchTable {
  BmagicMagic00800Handler handlers[3];
} BmagicMagic00800DispatchTable;

/*
 * Code pointer table at 0x801EEC40, the table this dispatcher copies:
 * entry 0 = 0x801EFAD4, entry 1 = 0x801EFC90, entry 2 = 0x801EFCFC.
 */
extern BmagicMagic00800DispatchTable D_801EEC40; /* @source 0x801EEC40 @kind data */

/* @source 0x801EFA74
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 selects an entry of the three-entry code pointer
 * table at 0x801EEC40 (0x801EFAD4, 0x801EFC90, 0x801EFCFC), and that entry is
 * invoked with no arguments. The three table words are copied into this
 * function's own 0x28-byte frame first, so the call target is read back from
 * the copy. It reads no other state and writes none.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic00800_801EFA74(void) {
  BmagicMagic00800DispatchTable handlers;

  handlers = D_801EEC40;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
