#include "bof3/bof3.h"

typedef void (*BmagicMagic06703Handler)(void);

typedef struct BmagicMagic06703DispatchTable {
  BmagicMagic06703Handler handlers[3];
} BmagicMagic06703DispatchTable;

/*
 * Code pointer run at 0x801EEC18, the table this dispatcher copies:
 * entry 0 = 0x801F03EC, entry 1 = 0x801F043C, entry 2 = 0x801F0498, the three
 * code blocks that follow this overlay's state-2 dispatchers. The run is the
 * tail of the overlay's leading data block, which no map row owns, so the
 * reference is bound by the target's generated undefined-symbol table.
 */
extern BmagicMagic06703DispatchTable D_801EEC18; /* @source 0x801EEC18 @kind data */

/* @source 0x801F038C
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its work byte at offset 2 selects an entry of the three-entry code pointer
 * run at 0x801EEC18 (0x801F03EC, 0x801F043C, 0x801F0498), and that entry is
 * invoked with no arguments. The three table words are copied into this
 * function's own 0x28-byte frame first (three lw from the run, three sw to
 * sp+0x10..0x18), so the call target is read back from the copy at
 * sp+0x10+index*4. It reads no other state and writes none.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerBmagicMagic06703_801F038C(void) {
  BmagicMagic06703DispatchTable handlers;

  handlers = D_801EEC18;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
