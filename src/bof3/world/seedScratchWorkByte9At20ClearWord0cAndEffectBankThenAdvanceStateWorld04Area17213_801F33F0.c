#include "bof3/bof3.h"

/* Published scratch work object. The original materialises this cell with its
 * own lui 0x1F80 plus a load at displacement 0x44 once for the byte seed and
 * again before the state byte is advanced. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared effect-bank byte at 0x8014832E, bound by the shared map. */
extern u8 D_8014832E; /* @source 0x8014832E @kind unknown */

/* @source 0x801F33F0
 * @behavior Entry 0 of the work state byte 1 handler run at 0x801F50A8. It
 *           seeds byte 0x09 of the work record published at the scratchpad
 *           pointer cell 0x1F800044 with 0x20 through one load of that cell,
 *           clears the record word at 0x0C on that same record, then advances
 *           the state byte at 0x01 of the record read through a fresh load of
 *           the cell and clears the shared effect-bank byte 0x8014832E. Takes
 *           no arguments and returns nothing; the body is a leaf with no frame
 *           and no saved register.
 *
 * Evidence: payload asm at 0x801F33F0 - lui $v1 of 0x1F80 with lw 0x44($v1),
 * addiu $v0,$zero,0x20, sb $v0,0x9($v1), a fresh lui $a0 plus lw 0x44($a0),
 * sw $zero,0xC($v1), lbu $v0,0x1($a0), nop, addiu $v0,$v0,1, sb $v0,0x1($a0),
 * lui $at of 0x8014832E with sb $zero in its delay, then jr $ra with a nop
 * delay slot; 0x3C bytes, so no trailing slack.
 *
 * Name evidence: the payload holds no jal to this address; its only in-image
 * reference is the word at 0x801F50A8, entry 0 of the code-pointer run based
 * there, which the dispatcher dispatchStateByte1Table50a8World04Area17213_
 * 801F33AC indexes with work record byte 0x01. The span therefore starts the
 * state byte 1 run, and the run's entry 1 advanceStepDrawTileUntilCountdown
 * ThenSeedStepsAndArmEffectWorld04Area17213_801F342C consumes the countdown byte
 * 0x09 and the step word 0x0C this entry seeds and cleared, which fixes the
 * roles written here. The statement order, operand forms and declaration
 * style follow the exact sibling setScratchWorkByte9ClearHalf2eAdvanceState
 * World04Area17213_801F3034 in this overlay, whose body differs in its stored
 * literal and its single stored field; the shared effect-bank clear takes the
 * spelling used by claimWorkRecordEveryFourthTickThenDrawSlotsAndAdvanceState
 * WhenEffectIdleWorld04Area17213_801F32E8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedScratchWorkByte9At20ClearWord0cAndEffectBankThenAdvanceStateWorld04Area17213_801F33F0(
    void) {
  u8* work;
  u8* next;

  work = D_1F800044;
  work[9] = 0x20;
  next = D_1F800044;
  *(u32*)(work + 0xc) = 0;
  next[1]++;
  D_8014832E = 0;
}
