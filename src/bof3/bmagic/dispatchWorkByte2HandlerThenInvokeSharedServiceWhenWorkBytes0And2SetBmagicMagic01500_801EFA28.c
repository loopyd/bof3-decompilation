#include "bof3/bof3.h"

typedef void (*BmagicMagic01500EightByte2Handler)(void);

typedef struct BmagicMagic01500EightByte2DispatchTable {
  BmagicMagic01500EightByte2Handler handlers[8];
} BmagicMagic01500EightByte2DispatchTable;

/*
 * Code pointer run at 0x801EEC34, the eight-entry table this dispatcher copies:
 * entry 0 = 0x801EFAE4, entry 1 = 0x801EFB90, entry 2 = 0x801EFBD8, entry 3 =
 * 0x801EFCC4, entry 4 = 0x801EFD7C, entry 5 = 0x801EFDE8, entry 6 = 0x801EFE84
 * and entry 7 = 0x801EFEFC, all function starts of this overlay. The run sits in
 * the overlay's leading data block, which no map row owns, so the reference is
 * bound by the target's generated undefined-symbol table.
 */
extern BmagicMagic01500EightByte2DispatchTable workByte2HandlerTableBmagicMagic01500_801EEC34; /* @source 0x801EEC34 @kind data */

/* Scratchpad work-object pointer cell, spelled as the named cell so the
 * assembler folds the constant address into each load (lui $vN,0x1F80 /
 * lw $vN,0x44($vN)). The original re-reads the cell after the indirect call
 * because the called handler may rewrite it, and a spelled-out constant address
 * folds to one value the compiler keeps in a callee-saved register across the
 * call, so the cell is named here instead of read through the scratchpad macro. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* File-local prototype: the shared main-executable service func_8014D290 is
 * declared per caller, not in a shared header (see the note in
 * include/bof3/ui/game00_internal.h); this caller passes no arguments. */
void func_8014D290(void);

/* @source 0x801EFA28
 * @behavior Work byte 2 step dispatcher of the overlay work object, reached as
 *           entry 0 of the work byte 1 handler run at 0x801F1630 (0x801EFA28,
 *           0x801EFF1C, 0x801F0334, 0x801F0C30). It copies the eight-entry work
 *           byte 2 handler table at 0x801EEC34 (0x801EFAE4, 0x801EFB90,
 *           0x801EFBD8, 0x801EFCC4, 0x801EFD7C, 0x801EFDE8, 0x801EFE84,
 *           0x801EFEFC) into its own 0x38-byte frame as eight inline word copies,
 *           so the call target is read back from the copy at 0x10($sp), then
 *           reads the scratchpad work-object pointer cell at 0x1F800044 and
 *           invokes the copied entry selected by that object's unsigned byte 2
 *           with no arguments and no bounds test. After the handler returns it
 *           re-reads the cell and, when the work object's byte 0 and byte 2 are
 *           both non-zero, invokes the shared main-executable service
 *           func_8014D290. It takes no arguments and returns nothing; the frame
 *           keeps the table at 0x10 and $ra at 0x30, and the first cell load is
 *           hoisted above the frame adjustment. It reads and writes no other
 *           state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeSharedServiceWhenWorkBytes0And2SetBmagicMagic01500_801EFA28(
    void) {
  BmagicMagic01500EightByte2DispatchTable handlers;
  u8 *work;

  handlers = workByte2HandlerTableBmagicMagic01500_801EEC34;
  handlers.handlers[D_1F800044[2]]();
  work = D_1F800044;
  if (work[0] != 0 && work[2] != 0) {
    func_8014D290();
  }
}
