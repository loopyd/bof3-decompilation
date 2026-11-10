#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell 0x1F800044, bound by this target's map row
 * g_battle_work. The original re-materializes the symbol for the second access
 * group (lui $v1,0x1F80 / lw $v1,0x44($v1)) because the byte store made through
 * the pointer the cell supplied may alias the cell itself; the exact sibling
 * incrementWorkByteAThenDecrementWorkByteBAndInvokeHelperWhenZeroBmagicMagic07903_801EF348
 * keeps the same reload spelling. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF8A0
 * @behavior Per-frame countdown step of the work object published at the
 *           scratchpad pointer cell 0x1F800044. It loads the cell once
 *           (lui $v1,0x1F80 / lw $v1,0x44($v1), empty load-delay slot),
 *           decrements the object byte 0xB at byte width (lbu $v0,0xB($v1) /
 *           addiu $v0,$v0,-1 / sb $v0,0xB($v1)) and compares the truncated
 *           stored value with zero (andi $v0,$v0,0xFF / bnez with an empty
 *           delay slot), so the byte is not re-read for the test. Only when
 *           that byte reaches zero it re-materializes the cell with its own
 *           lui/lw pair and increments the object byte 0x2 by one at byte
 *           width (lbu $v0,0x2($v1) / addiu $v0,$v0,1 / sb $v0,0x2($v1)).
 *           The 76-byte body has no frame, no call and no other store, so it
 *           is a leaf: jr $ra with an empty delay slot. No in-image pointer
 *           run, table or jal in this 6032-byte payload references 0x801EF8A0,
 *           so the entry is reached from outside the overlay and the name is
 *           read from the body rather than from a dispatch table. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByteBThenIncrementWorkByte2WhenZeroBmagicMagic08603_801EF8A0(void) {
  if (--g_battle_work[0xB] == 0) {
    g_battle_work[2]++;
  }
}
