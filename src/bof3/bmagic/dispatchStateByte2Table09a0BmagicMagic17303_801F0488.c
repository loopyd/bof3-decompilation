#include "bof3/bof3.h"

/* Code-pointer run at 0x801F09A0, the byte-2 handler table this dispatcher
 * indexes: entry 0 = 0x801F0540, entry 1 = 0x801F0648, entry 2 = 0x801F06B0,
 * entry 3 = 0x801F0700. It is the tail of the ten-word run that starts at
 * 0x801F0988, so the symbol is bound at the word the scaled index is added to
 * rather than at the run base. */
extern void (*D_801F09A0[])(void); /* @source 0x801F09A0 @kind data */

/* Global the overlay step rewrites around a handler call; nothing in this
 * payload reads the two literals back, so the cell keeps its address-anchored
 * name. */
extern volatile u32 D_801459F0; /* @source 0x801459F0 @kind unknown */

/* Scratchpad work-object pointer cell of this overlay, bound at 0x1F800044 by
 * this target's map. The original re-materialises the cell for the dispatch
 * index and again after the guard's two helper calls, so the two accesses are
 * fresh loads of the cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared main-executable helpers and the in-overlay step handler this span
 * calls; each is declared per caller, none of them is in a shared header. */
void func_800AF764(void); /* @source 0x800AF764 */
void func_801F075C(void); /* @source 0x801F075C */
void func_8014D290(void); /* @source 0x8014D290 */

/* @source 0x801F0488
 * @behavior Step wrapper around the byte-2 work handlers: publishes 0x800C3800
 *           into the global at 0x801459F0, reads the work object published at
 *           the scratchpad pointer cell 0x1F800044, scales that object's
 *           unsigned byte 2 by four and invokes the selected entry of the
 *           file-local handler table based at 0x801F09A0 with no arguments.
 *           After the handler returns it re-reads the pointer cell and, when
 *           the work object's byte 0 is non-zero, calls the shared
 *           main-executable helper func_800AF764 and the in-overlay step
 *           handler func_801F075C, re-reads the pointer cell once more and,
 *           when work byte 2 is below 3 in an unsigned compare, calls the
 *           shared main-executable service func_8014D290. It then publishes
 *           0x800D3800 into 0x801459F0. The first literal reaches the cell
 *           through its own lui/ori pair and a lui %hi / sw %lo store; the
 *           second is re-materialised in both guard delay slots and again on
 *           the fall-through after the service call before the join. The
 *           0x18-byte frame only keeps $ra at 0x10 across the calls.
 *
 *           The span is entry 1 of the ten-word code-pointer run at
 *           0x801F0988, the table the byte-1 dispatcher of this overlay
 *           indexes with work byte 1, so that table row proves the entry
 *           boundary and the no-argument ABI; no word of this payload
 *           jal-calls it. A byte search of every payload under out/binaries
 *           found no second copy of this 0xB8-byte span, so the clone lever was
 *           declined; the two exact lifts dispatchStateByte2Tablef8e0Bmagic
 *           Magic04500_801EEEE8 and dispatchStateByte2Table123cBmagic
 *           Magic11403_801F0D08 differ by relocation and supplied the
 *           table-symbol binding, the literal-publishing shape and the
 *           pointer-cell reload discipline as shape evidence only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchStateByte2Table09a0BmagicMagic17303_801F0488(void) {
  u8 *work;

  D_801459F0 = 0x800C3800;
  D_801F09A0[g_battle_work[2]]();
  work = g_battle_work;
  if (work[0] != 0) {
    func_800AF764();
    func_801F075C();
    work = g_battle_work;
    if (work[2] < 3) {
      func_8014D290();
    }
  }
  D_801459F0 = 0x800D3800;
}
