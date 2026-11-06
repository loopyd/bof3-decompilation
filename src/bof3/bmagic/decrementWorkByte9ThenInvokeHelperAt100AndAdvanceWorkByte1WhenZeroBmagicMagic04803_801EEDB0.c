#include "bof3/bof3.h"

/* Helper of the main executable at 0x8015D71C, outside this overlay's image.
 * The lift only calls it, so the prototype follows the call site: one
 * whole-word selector argument, no result used. This is the only reference to
 * the address in this target, so the generated undefined_funcs_auto.txt no
 * longer binds it after the handler moved to C; the target map row supplies the
 * weak address binding. */
void func_8015D71C(s32 arg0);

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers
 * and handlers and named the same way by the exact bmagic siblings. The
 * symbolic address folds into each load (lui $vN,0x1F80 / lw $vN,68($vN)), so
 * the original's two separate reads each re-materialise the base; the
 * constant-address accessor spelling of this cell instead keeps its address
 * live across the call, which costs a saved register and the original's
 * $ra slot. The plain cell is enough here because the call between the two
 * reads already forces the second one. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EEDB0
 * @behavior Step 1 of this overlay's work sequence: the handler the dispatcher
 *           at 0x801EEC10 selects through work byte 1, which the three-entry
 *           code pointer table at 0x801EEC04 binds at entry 1. It reads the
 *           scratchpad work-object pointer cell at 0x1F800044 once and
 *           decrements the work object's byte at offset 0x9 with a byte-width
 *           read-modify-write (lbu / addiu -1 / sb), then tests the truncated
 *           byte it just stored (andi 0xFF of the stored register, no second
 *           load of the cell). Only when that countdown reached zero does it
 *           invoke the main-executable helper at 0x8015D71C with the selector
 *           0x100, which is passed in the call's delay slot, re-read the
 *           pointer cell and increment the work object's byte at offset 0x1
 *           with a byte-width add, advancing the dispatch state to entry 2
 *           (0x801EEE10). The second cell read is required: the call may
 *           change the cell. The 0x18-byte frame only keeps $ra across that
 *           call. Takes no arguments, returns nothing and reads or writes no
 *           other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenInvokeHelperAt100AndAdvanceWorkByte1WhenZeroBmagicMagic04803_801EEDB0(void) {
  u8 *work;
  u8 countdown;

  work = g_battle_work;
  countdown = work[9] - 1;
  work[9] = countdown;
  if (countdown == 0) {
    func_8015D71C(0x100);
    g_battle_work[1]++;
  }
}
