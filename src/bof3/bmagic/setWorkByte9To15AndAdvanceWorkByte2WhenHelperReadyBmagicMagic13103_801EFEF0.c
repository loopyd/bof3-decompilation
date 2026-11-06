#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The cell is read once per access, so the original re-materializes
 * its address (lui $vN,0x1F80 / lw $vN,68($vN)) for each of the two accesses. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared readiness helper, outside this overlay's image. It reports a readiness
 * byte, which the exact main-executable lifts (clearWorkFlagsWhenReady,
 * stepWorkSequenceAndClearByte12BOnNonzero) read as a non-zero ready flag.
 * Splat binds the address through this target's generated
 * undefined_funcs_auto.txt, so no target-map row is needed. */
u8 func_8014DAEC(void);

/* @source 0x801EFEF0
 * @behavior Entry index 9 (the word at 0x801F1BE8) of the twelve-entry code
 * pointer run at 0x801F1BC4 that
 * dispatchWorkByte1HandlerBmagicMagic13103_801EF668 indexes with scratchpad
 * work-object byte 1, so it runs as the step handler for that selector while
 * work byte 2 is still zero. Per call it runs the shared readiness helper
 * func_8014DAEC and tests the returned byte zero-extended (andi 0xff, beqz);
 * the handler's only work happens once the helper reports ready: it
 * re-materializes the work-object pointer cell at 0x1F800044 and stores 0xF
 * into the work object's byte 9, then reads the cell again and increments the
 * work object's byte 2 with a byte-width add (lbu / addiu 1 / sb), which
 * advances dispatch to table entry 10 (0x801EFF48). The 0xF constant is
 * materialized ahead of the guard branch. The 0x18-byte frame only keeps $ra
 * at 0x10 across the helper call, and the function reads no other state, writes
 * no other state and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByte9To15AndAdvanceWorkByte2WhenHelperReadyBmagicMagic13103_801EFEF0(void) {
  if (func_8014DAEC() != 0u) {
    g_battle_work[9] = 0xF;
    g_battle_work[2]++;
  }
}
