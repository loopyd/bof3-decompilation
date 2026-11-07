#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays. The original re-materializes the cell (lui $vN,0x1F80 / lw
 * $vN,0x44($vN)) for the second read after the arm's byte store, so the cell
 * is reached through this target-map symbol rather than through a constant
 * address, which the compiler materializes once as lui/ori plus a zero
 * displacement load and then reuses for both reads. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF35C
 * @behavior Step handler 5 of the per-frame step run at 0x801EEC04, whose
 * first eight words the dispatcher at 0x801EEC24 copies into its frame and
 * indexes with the scratchpad work object's byte 0x1, so this body runs while
 * that step byte holds 5 and no jal in the image reaches this address. Reads
 * the work-object pointer cell at 0x1F800044 and, only when the work object's
 * byte at offset 0xB equals 6, stores 0x84 into that byte and then increments
 * the work object's byte at offset 0x1 with a byte-width add, which advances
 * the dispatcher to handler 6 at 0x801EF3A0. The increment goes through a
 * second read of the cell because the store through the first pointer may
 * alias it; the original materialises that second base with lui $v1,0x1F80 /
 * lw $v1,0x44($v1) instead of reusing the guard's $a0. The guard compares
 * against a materialized constant (addiu $v0,$zero,6 then bne) whose delay
 * slot holds the 0x84 store constant, which the guard's compare register no
 * longer needs. Leaf with no frame and no saved registers: takes no arguments,
 * returns nothing and reads or writes no other state. Statement order,
 * operand forms and cell spelling follow the exact sibling
 * setWorkByte9To8AndIncrementWorkByte1WhenWorkByteBZeroBmagicMagic07603_801EF11C
 * (emi/bmagic/magic076/03@0x801EF11C), whose 68 bytes are this function's with
 * guard constant 6, stored value 0x84 and store offset 0xB in place of a
 * plain zero test, 8 and 9.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByteBTo84AndIncrementWorkByte1WhenWorkByteBIs6BmagicMagic11303_801EF35C(void) {
  if (g_battle_work[0xB] == 6) {
    g_battle_work[0xB] = 0x84;
    g_battle_work[1]++;
  }
}
