#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. This target's map binds the cell, and the address is
 * re-materialized (lui $vN,0x1F80 / lw $vN,68($vN)) once per access, which is
 * how the original issues the three access groups of this body. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared function at 0x80155AF8, outside this overlay's image and called with
 * the work object's countdown byte through a signed byte view. This target's
 * generated undefined_funcs_auto.txt already binds the address, so the callee
 * keeps its raw name and takes no target-map row. */
extern void func_80155AF8(s8 arg0); /* @source 0x80155AF8 */

/* @source 0x801F0DD8
 * @behavior Handler step 1 of the four-word handler run at 0x801F1394 that the
 * dispatcher at 0x801F0D64 selects with the scratchpad work object's byte 0x2.
 * Reads the work-object pointer cell at 0x1F800044 and decrements that
 * object's byte at offset 0x9 with a byte-width add (lbu, addiu $v1,$v1,-1,
 * sb), then, only when the decremented value read back as a signed byte equals
 * -6, re-reads the cell and increments the work object's byte at offset 0x2
 * with a byte-width add (lbu, addiu $v0,$v0,1, sb through the same base).
 * Finally re-reads the cell a third time, loads byte 0x9 through a signed byte
 * view (lb) and passes it to the shared function at 0x80155AF8. The compare
 * constant is materialized in $v0 after the store, so the 0x18-byte frame only
 * keeps $ra across that call, whose delay slot is a nop; nothing is returned
 * and no other state is read or written. The statement order, the offset-local
 * spelling and the signed byte view follow the exact sibling
 * addWorkColorTripletBy16ThenDecrementGlobalWorkByteBAndAdvanceWorkByte2
 * BmagicMagic01500_801F1180 (emi/bmagic/magic015/00@0x801F1180), whose own
 * signed byte compare and byte increment toward byte 0x2 measure the same way.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementInvokeHelper(void) {
  s8 remaining;

  remaining = g_battle_work[9] - 1;
  g_battle_work[9] = remaining;
  if (remaining == -6) {
    g_battle_work[2]++;
  }
  func_80155AF8(((s8 *)g_battle_work)[9]);
}
