#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui 0x1F80 plus a load at displacement 0x44 for each of the three store
 * groups below, so the cell is its own fixed-address symbol rather than the
 * constant-address accessor, whose shared base would stay live across the
 * calls in a saved register and widen the frame. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

void claimFreeWorkRecordSeedPositionAndFadeWorld04Area18813_801F3DE0(void);

/* Per-frame passes this overlay's progress handlers run, still anchored to
 * their addresses. 0x801F44B4 is the 0x800E48C0-table draw pass: it asks for a
 * 0x380 tpage, sets the draw mode, primes a 0x18-byte buffer and runs 0x10
 * iterations of func_801F40F4 over the 0x2C-stride entries while advancing the
 * halfword at 0x800E48E4 by 0x10. 0x801F3E7C is the record pass over the
 * 0x800E4800 table that ticks and draws the active records. */
void drawEffectEntriesThenAdvanceAngle24By10World04Area18813_801F44B4(void);
void func_801F3E7C(void);

/* @source 0x801F364C
 * @behavior Overlay progress step handler. It reads the byte at offset 0x06 of
 * the work record published at the scratchpad pointer cell 0x1F800044 and
 * stores that byte plus one back, so the byte counts the frames this handler
 * runs; whenever the pre-increment value's low two bits are clear - every
 * fourth frame - it calls the work-record spawn 0x801F3DE0, which claims the
 * first free entry of the 0x18-byte-stride record table at 0x800E4800 and
 * seeds it from this work record. It then runs this overlay's two per-frame
 * passes 0x801F44B4 and 0x801F3E7C in that order and, re-reading the work
 * record through the cell each time, arms the countdown byte at offset 0x09
 * with 0x20 and advances the state byte at offset 0x01 by one, which moves the
 * overlay to the next progress step. It takes no arguments, returns nothing
 * and its 0x18-byte frame only holds $ra at 0x10 across the three calls.
 *
 * Evidence: payload asm at 0x801F364C - lui 0x1F80 plus lw 0x44 before the
 * frame is created, addiu $sp,-0x18, sw $ra,0x10($sp), lbu $v1,0x6($v0), nop,
 * addiu $a0,$v1,1, andi $v1,$v1,3, bnez to the join with sb $a0,0x6($v0) in
 * its delay slot, jal 0x801F3DE0 only on the fall-through, then jal 0x801F44B4
 * and jal 0x801F3E7C with nop delay slots, a fresh lui/lw pair of the same
 * cell, addiu $v0,$zero,0x20 with sb $v0,0x9($v1), a third lui/lw pair, lbu
 * $v0,0x1($v1), addiu $v0,$v0,1, lw $ra,0x10($sp), sb $v0,0x1($v1), jr $ra
 * with addiu $sp,$sp,0x18 in its delay slot.
 *
 * Name evidence: the only in-image reference to this address is the word at
 * 0x801F4E18, inside the seven-word code-pointer run at 0x801F4E0C that also
 * holds 0x801F3400, 0x801F3484, 0x801F3598, 0x801F36C4, 0x801F37CC and
 * 0x801F38C0; no in-payload code indexes that run, so the run names this
 * function's position among the overlay's progress handlers and the caller
 * stays outside the payload. The every-fourth-tick cadence is the one this
 * target's exact claimFreeWorkRecordSeedPositionAndFadeWorld04Area18813_801F3DE0
 * records for its four step-handler callers, and the state-byte advance is
 * what the byte-1 dispatch tables at 0x801F4DF4, 0x801F4DFC and 0x801F4E04
 * index. Work byte 0x09 is the countdown the sibling handlers
 * 0x801F3598 and 0x801F36C4 decrement before advancing byte 0x01, and 0x20 is
 * the value armed here.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimWorkRecordEveryFourthTickThenArmWorkByte9AndAdvanceWorkByte1World04Area18813_801F364C(
    void) {
  u8 tick;

  tick = D_1F800044[6]++;
  if ((tick & 3) == 0) {
    claimFreeWorkRecordSeedPositionAndFadeWorld04Area18813_801F3DE0();
  }
  drawEffectEntriesThenAdvanceAngle24By10World04Area18813_801F44B4();
  func_801F3E7C();
  D_1F800044[9] = 0x20;
  D_1F800044[1]++;
}
