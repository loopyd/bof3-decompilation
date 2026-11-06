#include "bof3/bof3.h"

/*
 * Code-pointer run at 0x801EF8E0, the handler table this dispatcher indexes:
 * entry 0 = 0x801EEF80, entry 1 = 0x801EF118, entry 2 = 0x801EF2B0, entry 3 =
 * 0x801EF3D8, entry 4 = 0x801EF4A4, entry 5 = 0x801EF608, entry 6 = 0x801EF724,
 * entry 7 = 0x801EF8A4. It is the tail of the ten-word run that starts at
 * 0x801EF8D8, so the symbol is bound at the word the scaled index is added to
 * rather than at the run base.
 */
extern void (*D_801EF8E0[])(void); /* @source 0x801EF8E0 @kind data */

/* Global the overlay step rewrites around a handler call; nothing in this
 * payload reads the two literals back, so the cell keeps its address-anchored
 * name. */
extern volatile u32 D_801459F0; /* @source 0x801459F0 @kind unknown */

/* Scratchpad work-object pointer cell of this overlay, read through the named
 * target symbol. The original re-materialises the cell for the dispatch index
 * and again after the indirect call, and one shared local cannot span the call.
 */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* File-local prototype: the shared main-executable service func_8014D290 is
 * declared per caller, not in a shared header (see the note in
 * include/bof3/ui/game00_internal.h); this caller passes no arguments. */
void func_8014D290(void);

/* @source 0x801EEEE8
 * @behavior Step wrapper around one overlay work handler: publishes 0x800C3800
 *           into the global at 0x801459F0, reads the work object published at
 *           the scratchpad pointer cell 0x1F800044, scales that object's
 *           unsigned byte 2 by four and invokes the selected entry of the
 *           eight-entry handler table based at 0x801EF8E0 with no arguments.
 *           After the handler returns it re-reads the pointer cell and, when
 *           the work object's byte 0 and byte 2 are both non-zero, runs the
 *           shared main-executable service func_8014D290. It then publishes
 *           0x800D3800 into 0x801459F0. Each literal reaches the cell through
 *           its own lui/ori pair and a lui %hi / sw %lo store, the second
 *           literal being re-materialised in both guard delay slots and again
 *           after the service call. The 0x18-byte frame only keeps $ra across
 *           the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2Tablef8e0BmagicMagic04500_801EEEE8(void) {
  u8 *work;

  D_801459F0 = 0x800C3800;
  D_801EF8E0[D_1F800044[2]]();
  work = D_1F800044;
  if (work[0] != 0 && work[2] != 0) {
    func_8014D290();
  }
  D_801459F0 = 0x800D3800;
}
