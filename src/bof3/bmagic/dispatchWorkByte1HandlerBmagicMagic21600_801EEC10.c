#include "bof3/bof3.h"

typedef void (*BmagicMagic21600Handler)(void);

typedef struct BmagicMagic21600DispatchTable {
  BmagicMagic21600Handler handlers[3];
} BmagicMagic21600DispatchTable;

/*
 * Code pointer table at 0x801EEC04, the table this dispatcher copies: entry 0
 * is 0x801EEC70, entry 1 is 0x801EECDC and entry 2 is 0x801EEF58, the three
 * code blocks that follow this overlay's entry point. The run sits in the
 * overlay's leading data block, which no map row owns, so the reference is
 * bound by the target's generated undefined-symbol table.
 */
extern BmagicMagic21600DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC10
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 selects an entry of the three-entry code pointer
 * table at 0x801EEC04 (0x801EEC70, 0x801EECDC, 0x801EEF58), and that entry is
 * invoked with no arguments. The three table words are copied into this
 * function's own 0x28-byte frame first, so the call target is read back from
 * the copy. It reads no other state and writes none.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic21600_801EEC10(void) {
  BmagicMagic21600DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
