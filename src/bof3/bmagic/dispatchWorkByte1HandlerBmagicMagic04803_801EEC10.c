#include "bof3/bof3.h"

typedef void (*BmagicMagic04803Handler)(void);

typedef struct BmagicMagic04803DispatchTable {
  BmagicMagic04803Handler handlers[3];
} BmagicMagic04803DispatchTable;

/*
 * Code pointer table at 0x801EEC04, the table this dispatcher copies:
 * entry 0 = 0x801EEC70, entry 1 = 0x801EEDB0, entry 2 = 0x801EEE10, the three
 * code blocks that follow this overlay's entry point. The run sits in the
 * overlay's leading data block, which no map row owns, so the reference is
 * bound by the target's generated undefined-symbol table.
 */
extern BmagicMagic04803DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC10
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 selects an entry of the three-entry code pointer
 * table at 0x801EEC04 (0x801EEC70, 0x801EEDB0, 0x801EEE10), and that entry is
 * invoked with no arguments. The three table words are copied into this
 * function's own 0x28-byte frame first, so the call target is read back from
 * the copy. It reads no other state and writes none.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic04803_801EEC10(void) {
  BmagicMagic04803DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
