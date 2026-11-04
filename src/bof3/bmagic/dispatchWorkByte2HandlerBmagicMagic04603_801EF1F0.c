#include "bof3/bof3.h"

typedef void (*BmagicMagic04603Handler)(void);

typedef struct BmagicMagic04603DispatchTable {
  BmagicMagic04603Handler handlers[3];
} BmagicMagic04603DispatchTable;

/*
 * Code pointer table at 0x801EEC10, the table this dispatcher copies:
 * entry 0 = 0x801EF250, entry 1 = 0x801EF2A8, entry 2 = 0x801EF304, the three
 * code blocks that follow this overlay's state-1 dispatcher. The run sits in the
 * overlay's leading data block, which no map row owns, so the reference is bound
 * by the target's generated undefined-symbol table.
 */
extern BmagicMagic04603DispatchTable D_801EEC10; /* @source 0x801EEC10 @kind data */

/* @source 0x801EF1F0
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read,
 * its work byte at offset 2 selects an entry of the three-entry code pointer
 * table at 0x801EEC10 (0x801EF250, 0x801EF2A8, 0x801EF304), and that entry is
 * invoked with no arguments. The three table words are copied into this
 * function's own 0x28-byte frame first, so the call target is read back from
 * the copy. Entry 0 is the work-record reset at 0x801EF250, entry 1 the work
 * byte 2 advance at 0x801EF2A8 and entry 2 the shared helper wrapper at
 * 0x801EF304. It reads no other state itself and writes none.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerBmagicMagic04603_801EF1F0(void) {
  BmagicMagic04603DispatchTable handlers;

  handlers = D_801EEC10;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
