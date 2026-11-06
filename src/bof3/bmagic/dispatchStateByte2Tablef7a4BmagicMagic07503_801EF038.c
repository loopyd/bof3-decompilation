#include "bof3/bof3.h"

/*
 * Code-pointer run at 0x801EF7A0, the handler table this dispatcher indexes:
 * entry 0 = 0x801EF038 (this function, selected by the work byte 1 dispatcher
 * at 0x801EEFF4), entry 1 = 0x801EF0D0, entry 2 = 0x801EF2A8. The run is the
 * last twelve bytes of this payload. The symbol is bound at the word the scaled
 * index is added to (0x801EF7A4), not at the run base.
 */
extern void (*D_801EF7A4[])(void); /* @source 0x801EF7A4 @kind data */

/* Global the overlay step rewrites around a handler call; nothing in this
 * payload reads the two literals back, so the cell keeps its address-anchored
 * name. */
extern volatile u32 D_801459F0; /* @source 0x801459F0 @kind unknown */

/* Scratchpad work-object pointer cell. The original re-materialises this cell
 * for the dispatch index and again after the indirect call, so it is read
 * through the named target symbol instead of the constant-address accessor. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* File-local prototype: the shared main-executable service func_8014D290 is
 * declared per caller, not in a shared header (see the note in
 * include/bof3/ui/game00_internal.h); this caller passes no arguments. */
void func_8014D290(void);

/* @source 0x801EF038
 * @behavior Step wrapper around one overlay work handler: publishes 0x800C3800
 *           into the global at 0x801459F0, reads the work object published at
 *           the scratchpad pointer cell 0x1F800044, scales that object's
 *           unsigned byte 2 by four and invokes the selected entry of the
 *           handler run based at 0x801EF7A4 (0x801EF0D0, 0x801EF2A8) with no
 *           arguments. After the handler returns it re-reads the pointer cell
 *           and, when the work object's byte 0 and byte 2 are both non-zero,
 *           runs the shared main-executable service func_8014D290. It then
 *           publishes 0x800D3800 into 0x801459F0. Each literal reaches the
 *           cell through its own lui/ori pair and a lui %hi / sw %lo store, the
 *           second literal being re-materialised in both guard delay slots and
 *           again after the service call. The 0x18-byte frame only keeps $ra
 *           across the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2Tablef7a4BmagicMagic07503_801EF038(void) {
  u8 *work;

  D_801459F0 = 0x800C3800;
  D_801EF7A4[g_battle_work[2]]();
  work = g_battle_work;
  if (work[0] != 0 && work[2] != 0) {
    func_8014D290();
  }
  D_801459F0 = 0x800D3800;
}
