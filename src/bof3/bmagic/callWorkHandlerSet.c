#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell bound at 0x1F800044 by this target's
 * exact lifts, so the symbolic address folds into each load. The original
 * re-reads the cell after the indirect call because the dispatched handler may
 * republish the pointer, so one shared local cannot span the call. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Work byte 2 handler table this overlay dispatches through, indexed at its own
 * base 0x801EFC44: entry 0 = 0x801EF47C, entry 1 = 0x801EF628, entry 2 =
 * 0x801EF6E0. That base is the sixth word of the nine-word code-pointer run
 * this target stores at 0x801EFC2C, which Splat places as the single `textbin`
 * row workByte1HandlerTable, so the extern is declared at the indexed word and
 * the run stays one map row; the generated undefined_syms_auto.txt row for
 * 0x801EFC44 already names this word. */
extern void (*D_801EFC44[])(void); /* @source 0x801EFC44 @kind data */

/* This overlay's second draw step at 0x801EF7A8, still on its own `asm` row.
 * It selects a graph-type draw mode (0x125 for types 1 and 2, 0x55 otherwise),
 * arms SetDrawMode at g_PrimCursor and continues with a primitive packet, so
 * the body below only calls it and its role stays address-anchored. */
extern void func_801EF7A8(void); /* @source 0x801EF7A8 */

/* @source 0x801EF404
 * @behavior Work byte 2 step handler of the overlay work object: reads the
 *   scratchpad work-object pointer cell at 0x1F800044 before its 0x18-byte
 *   frame is created and invokes the code pointer that the work object's
 *   unsigned byte 2, scaled by four, selects in the handler table based at
 *   0x801EFC44, with no arguments and no bounds test. After the handler returns
 *   it re-reads the cell and, when the work object's byte 0 and byte 2 are both
 *   non-zero, calls the overlay draw step at 0x801EF7A8. The handler table is
 *   the tail of the nine-word code-pointer run at 0x801EFC2C, and this span is
 *   word 5 of that run, so the payload holds no jal caller for it. The cell is
 *   re-read rather than kept live because the dispatched handler may republish
 *   it. Takes no arguments, returns nothing and reads or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callWorkHandlerSet(
    void) {
  u8 *work;

  D_801EFC44[g_battle_work[2]]();
  work = g_battle_work;
  if (work[0] != 0 && work[2] != 0) {
    func_801EF7A8();
  }
}
