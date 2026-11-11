#include "bof3/bof3.h"

typedef void (*BmagicMagic00403Byte2Handler)(void);

typedef struct BmagicMagic00403Byte2DispatchTable {
  BmagicMagic00403Byte2Handler handlers[4];
} BmagicMagic00403Byte2DispatchTable;

/*
 * Code pointer run at 0x801EEC18, the four-entry table this dispatcher copies:
 * entry 0 = 0x801EF9A0, entry 1 = 0x801EF9E8, entry 2 = 0x801EFA70,
 * entry 3 = 0x801EFAD4, the four work byte 2 step handlers of this overlay. The
 * run is the last four words of the overlay's leading table block, the words
 * that follow the five-entry run at 0x801EEC04, and no map row owns it, so the
 * reference is bound by the target's generated undefined-symbol table.
 */
extern BmagicMagic00403Byte2DispatchTable D_801EEC18; /* @source 0x801EEC18 @kind data */

/* Scratchpad work-object pointer cell, the named cell this target's map already
 * binds. The original re-materialises it for the dispatch index and again after
 * the indirect call, because the called handler may rewrite it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* File-local prototype: the shared main-executable service func_8014D290 is
 * declared per caller, not in a shared header (see the note in
 * include/bof3/ui/game00_internal.h); this caller passes no arguments. */
void func_8014D290(void);

/* @source 0x801EF910
 * @behavior Work byte 2 step dispatcher of the overlay work object, reached as
 *           entry 0 of the overlay-local work byte 1 code pointer table at
 *           0x801EFE58 that 0x801EF8CC dispatches (0x801EF910, 0x801EFAF4,
 *           0x801EFBA8, 0x801EFBEC, 0x801EFD84). It copies the four-entry work
 *           byte 2 handler table at 0x801EEC18 (0x801EF9A0, 0x801EF9E8,
 *           0x801EFA70, 0x801EFAD4) into its own 0x28-byte frame as four word
 *           copies, so the call target is read back from the copy at 0x10($sp),
 *           then reads the scratchpad work-object pointer cell at 0x1F800044
 *           and invokes the copied entry selected by that object's unsigned
 *           byte 2, with no arguments and no bounds test. After the handler
 *           returns it re-reads the pointer cell and, when the work object's
 *           flags byte 0 is non-zero, runs the shared main-executable service
 *           func_8014D290. It takes no arguments and returns nothing; the frame
 *           keeps the table at 0x10 and $ra at 0x20, and the first cell load is
 *           hoisted above the frame adjustment. It reads and writes no other
 *           state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeSharedServiceWhenWorkByte0SetBmagicMagic00403_801EF910(
    void) {
  BmagicMagic00403Byte2DispatchTable handlers;

  handlers = D_801EEC18;
  handlers.handlers[g_battle_work[2]]();
  if (g_battle_work[0] != 0u) {
    func_8014D290();
  }
}
