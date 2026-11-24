#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell shared by the battle and bmagic overlays;
 * this target's map binds it at 0x1F800044 as g_battle_work, the same binding
 * the exact sibling decrementWorkByteBThenIncrementWorkByte2WhenZeroBmagic
 * Magic08603_801EF8A0 uses. The original re-materializes the cell for the
 * taken-path access group (lui $a0,0x1F80 / lw $a0,0x44($a0)) because the byte
 * store made through the pointer the cell first supplied may alias the cell
 * itself, and that one reload serves every store on the taken path. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F0100
 * @behavior Countdown step of the work object published at the scratchpad
 *           pointer cell 0x1F800044. It loads the cell once (lui $v1,0x1F80 /
 *           lw $v1,0x44($v1), empty load-delay slot), decrements the object
 *           byte 0x9 by one at byte width (lbu $v0,0x9($v1) / addiu
 *           $v0,$v0,-1 / sb $v0,0x9($v1)) and tests the truncated stored value
 *           with andi $v0,$v0,0xFF / bnez, whose delay slot carries
 *           addiu $v0,$zero,0x20, so the byte is not re-read for the test.
 *           Only when that byte reaches zero does it re-materialize the cell
 *           with its own lui/lw pair into $a0, seed word 0x14 with 0x20
 *           (sw $v0,0x14($a0)), seed word 0x20 with -1 (addiu $v1,$zero,-1 /
 *           sw $v1,0x20($a0)) and advance the object byte 0x2 by one at byte
 *           width (lbu $v0,0x2($a0) / addiu $v0,$v0,1 / sb $v0,0x2($a0)).
 *           The 84-byte body has no frame and no call, so it is a leaf: jr $ra
 *           with an empty delay slot. Word 1 of the pointer run
 *           tableHandlerByte2C at 0x801F0384 holds 0x801F0100, so the
 *           entry is reached through that table and no jal in this 6032-byte
 *           payload names it. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementSeedZero(
    void) {
  u8 *work;

  if (--g_battle_work[0x9] == 0) {
    work = g_battle_work;
    *(s32 *)(work + 0x14) = 0x20;
    *(s32 *)(work + 0x20) = -1;
    work[0x2]++;
  }
}
