#include "bof3/bof3.h"

typedef void (*BmagicMagic15103Handler)(void);

typedef struct BmagicMagic15103DispatchTable {
  BmagicMagic15103Handler handlers[6];
} BmagicMagic15103DispatchTable;

/*
 * Code pointer table at 0x801EEC38, the table this dispatcher copies:
 * entry 0 = 0x801EFAAC, entry 1 = 0x801EFBAC, entry 2 = 0x801EFC68,
 * entry 3 = 0x801EFD24, entry 4 = 0x801EFD80, entry 5 = 0x801EFDC0, the six
 * code blocks that follow this dispatcher. The run sits in the overlay's leading
 * data block, which no map row owns, so the reference is bound by the target's
 * generated undefined-symbol table.
 */
extern BmagicMagic15103DispatchTable D_801EEC38; /* @source 0x801EEC38 @kind data */

/* @source 0x801EFA34
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its state byte at offset 2 selects an entry of the six-entry code pointer
 * table at 0x801EEC38 (0x801EFAAC, 0x801EFBAC, 0x801EFC68, 0x801EFD24,
 * 0x801EFD80, 0x801EFDC0), and that entry is invoked with no arguments. The six
 * table words are copied into this function's own 0x30-byte frame first, so the
 * call target is read back from the copy. It reads no other state and writes
 * none.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerBmagicMagic15103_801EFA34(void) {
  BmagicMagic15103DispatchTable handlers;

  handlers = D_801EEC38;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
