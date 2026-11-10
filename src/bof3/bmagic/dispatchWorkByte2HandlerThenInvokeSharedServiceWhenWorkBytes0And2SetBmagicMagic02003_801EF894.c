#include "bof3/bof3.h"

typedef void (*BmagicMagic02003Byte2Handler)(void);

typedef struct BmagicMagic02003Byte2DispatchTable {
  BmagicMagic02003Byte2Handler handlers[3];
} BmagicMagic02003Byte2DispatchTable;

/*
 * Code pointer run at 0x801EEC3C, the three-entry table this dispatcher copies:
 * entry 0 = 0x801EF928, entry 1 = 0x801EFA70, entry 2 = 0x801EFAD4, the three
 * work byte 2 step handlers of this overlay. The run sits in the overlay's
 * leading data block, which no map row owns, so the reference is bound by the
 * target's generated undefined-symbol table. The run continues past these three
 * words; this extern covers only the copied prefix.
 */
extern BmagicMagic02003Byte2DispatchTable D_801EEC3C; /* @source 0x801EEC3C @kind data */

/* Scratchpad work-object pointer cell, spelled the same named cell the exact
 * lifts of this target already use. A named cell symbol lets the assembler fold
 * the constant address into each load (lui $vN,0x1F80 / lw $vN,0x44($vN)), and
 * the original re-reads the cell after the indirect call because the called
 * handler may rewrite it. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* File-local prototype: the shared main-executable service func_8014D290 is
 * declared per caller, not in a shared header (see the note in
 * include/bof3/ui/game00_internal.h); this caller passes no arguments. */
void func_8014D290(void);

/* @source 0x801EF894
 * @behavior Work byte 2 step dispatcher of the overlay work object, reached as
 *           entry 1 of the overlay's outer code pointer run at 0x801EFEB8
 *           (0x801EF3DC, 0x801EF894, 0x801EFAF4). It copies the three-entry work
 *           byte 2 handler table at 0x801EEC3C (0x801EF928, 0x801EFA70,
 *           0x801EFAD4) into its own 0x28-byte frame as three word copies, so the
 *           call target is read back from the copy at 0x10($sp), then reads the
 *           scratchpad work-object pointer cell at 0x1F800044 and invokes the
 *           copied entry selected by that object's unsigned byte 2 with no
 *           arguments and no bounds test. After the handler returns it re-reads
 *           the cell and, when the work object's byte 0 and byte 2 are both
 *           non-zero, invokes the shared main-executable service func_8014D290.
 *           It takes no arguments and returns nothing; the frame keeps the table
 *           at 0x10 and $ra at 0x20, and the first cell load is hoisted above
 *           the frame adjustment. It reads and writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeSharedServiceWhenWorkBytes0And2SetBmagicMagic02003_801EF894(
    void) {
  BmagicMagic02003Byte2DispatchTable handlers;
  u8 *work;

  handlers = D_801EEC3C;
  handlers.handlers[D_1F800044[2]]();
  work = D_1F800044;
  if (work[0] != 0 && work[2] != 0) {
    func_8014D290();
  }
}
