#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui 0x1F80 plus a load at displacement 0x44 once before the frame is created
 * and again for each of the two tail store groups, so the cell is read through
 * this target's map name rather than the constant-address accessor, whose
 * shared base would stay live across the calls in a saved register and widen
 * the frame. */
extern u8* scratchWorkPointerAt1f800044; /* @source 0x1F800044 @kind data */

void claimFreeWorkRecordSeedPositionAndFadeWorld02Area09813_801F526C(void);

/* Per-frame passes this overlay's progress handlers run, still anchored to
 * their addresses. 0x801F5940 is the draw pass over the 0x800E48C0 table: it
 * asks for the 0x380 tpage with GetTPage(0, 1, 0x380, 0x100) and walks that
 * table's entries. 0x801F5308 is the record pass over the 0x800E4800 table: it
 * primes a stack scratch word through 0x801AFE18 and ticks every active entry,
 * decrementing the byte at +0x00 and the halfword at +0x12 of each. This
 * handler runs the draw pass first and the record tick pass second, the order
 * the payload's own two jal words give. */
void func_801F5940(void);
void func_801F5308(void);

/* @source 0x801F3B5C
 * @behavior Overlay progress step handler, entry 5 of the nine-word handler
 *           row based at 0x801F5F88. It reads the frame byte at +0x06 of the
 *           work record published at the scratchpad pointer cell 0x1F800044 and
 *           stores that byte plus one back, so the byte counts the frames this
 *           handler runs; whenever the pre-increment value has its low two bits
 *           clear - every fourth frame - it calls the work-record spawn
 *           0x801F526C, which claims the first free entry of the
 *           0x18-byte-stride record table at 0x800E4800 and seeds it from this
 *           work record. It then runs this overlay's per-frame passes, the
 *           0x800E48C0 draw pass 0x801F5940 and the 0x800E4800 record tick pass
 *           0x801F5308, in that order. Next, re-reading the work record through
 *           the cell, it arms the countdown byte at +0x09 with 0x20, and then,
 *           re-reading the record through the cell once more, advances the state
 *           byte at +0x01 by one, which hands the record to entry 6 of the row,
 *           0x801F3BD4. It takes no arguments, returns nothing and its
 *           0x18-byte frame only holds $ra at 0x10 across the three calls.
 *
 * Evidence: payload asm at 0x801F3B5C - lui 0x1F80 plus lw 0x44 before the
 * frame is created, addiu $sp,-0x18, sw $ra,0x10($sp), lbu $v1,0x6($v0), nop,
 * addiu $a0,$v1,1, andi $v1,$v1,3, bnez to the join with sb $a0,0x6($v0) in its
 * delay slot, jal 0x801F526C only on the fall-through, then jal 0x801F5940 and
 * jal 0x801F5308 with nop delay slots, a fresh lui/lw pair of the same cell,
 * addiu $v0,$zero,0x20 with sb $v0,0x9($v1), a third lui/lw pair, lbu
 * $v0,0x1($v1), nop, addiu $v0,$v0,1, lw $ra,0x10($sp), sb $v0,0x1($v1), jr
 * $ra with addiu $sp,$sp,0x18 in its delay slot.
 *
 * Name evidence: the payload word at 0x801F5F9C is the sixth entry, index 5, of
 * the nine-word handler row based at 0x801F5F88 (0x801F3814, 0x801F3878,
 * 0x801F3910, 0x801F3994, 0x801F3AA8, 0x801F3B5C, 0x801F3BD4, 0x801F3CDC,
 * 0x801F3D98; the next dispatcher's row starts at 0x801F5FAC). The exact
 * dispatcher dispatchStateByte1Table5f88World02Area09813_801F37D0 scales the
 * work record's state byte at +0x01 by four, loads that word from the row and
 * jalrs it, so the row fixes this function as the state-5 progress handler and
 * its caller stays outside the payload. Entry 6 of the same row is this
 * target's exact
 * claimRecordEveryFourthTickRunPassesDrawQuadsIntegrateMotionReloadCountdownAndAdvanceStateWorld02Area09813_801F3BD4,
 * which this handler hands the record to through the +0x01 advance, and entry 4
 * is 0x801F3AA8, whose frame counter, every-fourth-tick spawn and tail call
 * pair carry the same cadence block as this span. The every-fourth-frame
 * cadence, the state-byte-1 advance and the countdown byte at +0x09 are the
 * roles this target's exact
 * claimFreeWorkRecordSeedPositionAndFadeWorld02Area09813_801F526C records for
 * its step-handler callers.
 *
 * Shape evidence: this 120-byte span, payload offsets 0xF5C-0xFD3, is
 * byte-identical to the 120-byte span at payload offset 0xA4C of
 * emi/world04/area188/13, which shares this target's 0x801F2C00 load address
 * and whose span is the already-exact lift
 * claimWorkRecordEveryFourthTickThenArmWorkByte9AndAdvanceWorkByte1World04Area18813_801F364C,
 * except its three jal own-immediates - 0x0C07CF78, 0x0C07D12D and 0x0C07CF9F
 * there, whose targets 0x801F3DE0, 0x801F44B4 and 0x801F3E7C are that overlay's
 * spawn, its 0x800E48C0 draw pass and its 0x800E4800 record tick pass - against
 * 0x0C07D49B, 0x0C07D650 and 0x0C07D4C2 here, whose targets 0x801F526C,
 * 0x801F5940 and 0x801F5308 are this overlay's spawn and the same two passes in
 * the same order. Every other word is equal, so one body carries both rows:
 * that twin supplies the statement order, the operand forms and the offset
 * spelling, and this function's three call targets and its cell binding are
 * written from this span's own bytes and this target's own map.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimRecordEveryFourthTickRunPassesArmWorkCountdown20AndAdvanceStateWorld02Area09813_801F3B5C(
    void) {
  u8 tick;

  tick = scratchWorkPointerAt1f800044[6]++;
  if ((tick & 3) == 0) {
    claimFreeWorkRecordSeedPositionAndFadeWorld02Area09813_801F526C();
  }
  func_801F5940();
  func_801F5308();
  scratchWorkPointerAt1f800044[9] = 0x20;
  scratchWorkPointerAt1f800044[1]++;
}
