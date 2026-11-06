#include "bof3/bof3.h"

typedef void (*BmagicMagic02003Handler)(void);

typedef struct BmagicMagic02003DispatchTable {
  BmagicMagic02003Handler handlers[6];
} BmagicMagic02003DispatchTable;

/*
 * Code pointer table at 0x801EEC04, the six-entry table this dispatcher
 * copies: entry 0 = 0x801EECC0, entry 1 = 0x801EF0F8, entry 2 = 0x801EF234,
 * entry 3 = 0x801EF2A8, entry 4 = 0x801EF31C, entry 5 = 0x801EF350, the six
 * code blocks that follow this overlay's entry point. The run sits in the
 * overlay's leading data block, which no map row owns, so the reference is
 * bound by the target's generated undefined-symbol table. The run continues
 * past these six words; this extern covers only the copied prefix.
 */
extern BmagicMagic02003DispatchTable D_801EEC04; /* @source 0x801EEC04 @kind data */

/* @source 0x801EEC48
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 1 selects an entry of the six-entry code pointer
 * table at 0x801EEC04 (0x801EECC0, 0x801EF0F8, 0x801EF234, 0x801EF2A8,
 * 0x801EF31C, 0x801EF350), and that entry is invoked with no arguments. The six
 * table words are copied into this function's own 0x30-byte frame first, so the
 * call target is read back from the copy. It reads no other state and writes
 * none, and no word of this payload holds its own address, so it is an overlay
 * entry reached from outside the payload; its prologue and return path alone
 * place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic02003_801EEC48(void) {
  BmagicMagic02003DispatchTable handlers;

  handlers = D_801EEC04;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
