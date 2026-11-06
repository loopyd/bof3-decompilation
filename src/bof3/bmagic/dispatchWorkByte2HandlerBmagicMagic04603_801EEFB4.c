#include "bof3/bof3.h"

typedef void (*BmagicMagic04603Handler)(void);

typedef struct BmagicMagic04603DispatchTable {
  BmagicMagic04603Handler handlers[3];
} BmagicMagic04603DispatchTable;

/*
 * Code pointer table at 0x801EEC04, the table this dispatcher copies:
 * entry 0 = 0x801EF014, entry 1 = 0x801EF090, entry 2 = 0x801EF1A0, the three
 * code blocks that follow this dispatcher. The run sits in the overlay's
 * leading data block, which no map row owns, so the reference is bound by the
 * target's generated undefined-symbol table.
 */
extern BmagicMagic04603DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEFB4
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its work byte at offset 2 selects an entry of the three-entry code pointer
 * table at 0x801EEC04, and that entry is invoked with no arguments. The three
 * table words are copied into this function's own 0x28-byte frame first, so
 * the call target is read back from the copy. The index is not bounds-checked.
 * Entry 0 is the work-record handler at 0x801EF014, entry 1 the block at
 * 0x801EF090 and entry 2 the block at 0x801EF1A0. It reads no other state
 * itself and writes none. Its only in-image reference is the word 0x801EEFB4 at
 * 0x801EFF34, entry 0 of this overlay's handler run, so the step dispatcher
 * that indexes that run is what calls it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerBmagicMagic04603_801EEFB4(void) {
  BmagicMagic04603DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
