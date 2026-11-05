#include "bof3/bof3.h"

typedef void (*BmagicMagic15103Handler)(void);

typedef struct BmagicMagic15103DispatchTable {
  BmagicMagic15103Handler handlers[3];
} BmagicMagic15103DispatchTable;

/*
 * Code pointer table at 0x801EEC04, the table this dispatcher copies:
 * entry 0 = 0x801EECB0, entry 1 = 0x801EEDE0, entry 2 = 0x801EF938, the three
 * code blocks that follow this dispatcher. The run sits in the overlay's leading
 * data block, which no map row owns, so the reference is bound by the target's
 * generated undefined-symbol table.
 */
extern BmagicMagic15103DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC50
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 selects an entry of the three-entry code pointer
 * table at 0x801EEC04 (0x801EECB0, 0x801EEDE0, 0x801EF938), and that entry is
 * invoked with no arguments. The three table words are copied into this
 * function's own 0x28-byte frame first, so the call target is read back from
 * the copy. It reads no other state and writes none.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic15103_801EEC50(void) {
  BmagicMagic15103DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
