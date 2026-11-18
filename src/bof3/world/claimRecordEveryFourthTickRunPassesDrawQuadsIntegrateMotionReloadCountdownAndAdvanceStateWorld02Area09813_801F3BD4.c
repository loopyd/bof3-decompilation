#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui 0x1F80 plus a load at displacement 0x44 before the frame is created, then
 * again for the render call's argument pair, once for the motion block and once
 * for each of the tail's three byte accesses, so the cell is read through this
 * target's map name rather than the constant-address accessor, whose shared
 * base would stay live across the calls in a saved register and widen the
 * frame. */
extern u8* scratchWorkPointerAt1f800044; /* @source 0x1F800044 @kind data */

void claimFreeWorkRecordSeedPositionAndFadeWorld02Area09813_801F526C(void);

/* Per-frame passes this overlay's progress handlers run, still anchored to
 * their addresses. 0x801F5308 is the record pass over the 0x800E4800 table
 * that ticks the active records; it is the call the step handlers of this row
 * make right after the spawn, and the exact
 * claimFreeWorkRecordSeedPositionAndFadeWorld02Area09813_801F526C records it
 * as the pass that reads and clears the claim flag this handler's spawn sets.
 * 0x801F5940 is the pass that follows it here and in the sibling handlers
 * 0x801F3AA8 and 0x801F3B5C of the same row. */
void func_801F5308(void);
void func_801F5940(void);

/* Overlay quad renderer of this area, still anchored to its address: it primes
 * a four-vertex semi-transparent polygon through the shared primitive cursor,
 * takes the work record's three-word triple at +0x18 as arg0 and the triple at
 * +0x34 as arg1, and uses the byte handed as arg2 as the channel mask colour
 * it stores into the primitive. */
void func_801F48E8(void* arg0, void* arg1, u8 channels);

/* @source 0x801F3BD4
 * @behavior Overlay progress step handler, entry 6 of the nine-word handler row
 *           based at 0x801F5F88. It reads the frame byte at +0x06 of the work
 *           record published at the scratchpad pointer cell 0x1F800044 and
 *           stores that byte plus one back, so the byte counts the frames this
 *           handler runs; whenever the pre-increment value has its low two bits
 *           clear - every fourth frame - it calls the work-record spawn
 *           0x801F526C, which claims the first free entry of the
 *           0x18-byte-stride record table at 0x800E4800 and seeds it from this
 *           work record. It then runs this overlay's two per-frame passes
 *           0x801F5308 and 0x801F5940 in that order, re-reads the work record
 *           through the cell and renders it through the overlay quad renderer
 *           0x801F48E8 with the record triple at +0x18, the triple at +0x34 and
 *           the channel mask 0x40. Next, through one word view of the work
 *           pointer, it subtracts 0x800 from the word at +0x0C and 0x200 from
 *           the word at +0x10, adds each updated word into the word that
 *           follows its own triple - +0x0C into +0x18 and +0x10 into +0x1C, the
 *           triple the renderer just consumed and the word behind it - and
 *           subtracts 4 from the work byte at +0x03. Finally it decrements the
 *           countdown byte at +0x09 and, on the frame that countdown reaches
 *           zero, reloads it with 0x80 and advances the state byte at +0x01 by
 *           one, which hands the record to the next handler of this row. It
 *           takes no arguments, returns nothing and its 0x18-byte frame only
 *           holds $ra at 0x10 across the four calls.
 *
 * Evidence: payload asm at 0x801F3BD4 - lui 0x1F80 plus lw 0x44 before the
 * frame is created, addiu $sp,-0x18, sw $ra,0x10($sp), lbu $v1,0x6($v0), nop,
 * addiu $a0,$v1,1, andi $v1,$v1,3, bnez to the join with sb $a0,0x6($v0) in its
 * delay slot, jal 0x801F526C only on the fall-through, then jal 0x801F5308 and
 * jal 0x801F5940 with nop delay slots, a fresh lui/lw pair of the same cell
 * into $a1 with addiu $a2,$zero,0x40 and addiu $a0,$a1,0x18, jal 0x801F48E8
 * with addiu $a1,$a1,0x34 in its delay slot, a third lui/lw pair whose base
 * $v1 stays live across the motion block: lw 0x10, lw 0x0C, addiu -0x200, sw
 * 0x10, lw 0x18, addiu -0x800, sw 0x0C, lbu 0x3, lw 0x0C, addiu -0x4, sb 0x3,
 * lw 0x1C, addu, sw 0x18, lw 0x10, addu, sw 0x1C, then a fourth lui/lw pair
 * with lbu 0x9, addiu $v0,$v0,-1, sb $v0,0x9($a1) followed by andi
 * $v0,$v0,0xFF and bnez to the epilogue, and on the expiry path a fifth lui/lw
 * pair storing addiu $v0,$zero,0x80 at 0x9 and a sixth reading, advancing and
 * storing back the byte at 0x1, then lw $ra,0x10($sp), addiu $sp,$sp,0x18, jr
 * $ra.
 *
 * Name evidence: the payload word at 0x801F5FA0 is the seventh entry of the
 * nine-word handler row based at 0x801F5F88 (0x801F3814, 0x801F3878,
 * 0x801F3910, 0x801F3994, 0x801F3AA8, 0x801F3B5C, 0x801F3BD4, 0x801F3CDC,
 * 0x801F3D98; the next dispatcher's row starts at 0x801F5FAC). The exact
 * dispatcher dispatchStateByte1Table5f88World02Area09813_801F37D0 scales the
 * work record's state byte at +0x01 by four, loads that word from the row and
 * jalrs it, so the row fixes this function as the state-6 progress handler and
 * its caller stays outside the payload. Entry 0 of the same row is this
 * target's exact seedWorkWords64AndAdvanceStateWorld02Area09813_801F3814. The
 * every-fourth-frame cadence, the state-byte-1 advance and the countdown byte
 * at +0x09 are the roles this target's exact
 * claimFreeWorkRecordSeedPositionAndFadeWorld02Area09813_801F526C records for
 * its step-handler callers. Nothing in this payload reads the work byte at
 * +0x03, so its role stays described by offset.
 *
 * Shape evidence: the 264-byte span is byte-identical to the asm span
 * func_801F36C4 of emi/world04/area188/13 (payload offsets 0xFD4 here and
 * 0xAC4 there, both 0x108 long) except its four jal own-immediates - 0x0807D498,
 * 0x0807D4C0, 0x0807D650 and 0x0807D238 there against 0x0807D49B, 0x0807D4C2,
 * 0x0807D650 and 0x0807D23A here, whose targets 0x801F3DE0, 0x801F3E7C,
 * 0x801F44B4 and 0x801F3ADC are that overlay's own spawn, passes and renderer -
 * and the one reload immediate at +0xC8, 0x10 there against 0x80 here. Every
 * other word, including the 0x200, 0x800, 4 and 0x40 literals and every
 * relative branch, is equal, so one body carries both rows. That twin is still
 * an asm row, so it supplied shape and naming, not a clone: the statement order,
 * operand forms and offset spelling of the cadence block, of the render call's
 * argument pair and of the countdown tail come from the exact handler lifts
 * claimWorkRecordEveryFourthTickThenArmWorkByte9AndAdvanceWorkByte1World04Area18813_801F364C
 * and
 * spawnRecordEveryFourthTickRunPassesDrawShapeThenOnProgress3CountdownQueueCue216SetByte3To80AndAdvanceWorkByte1World04Area18813_801F37CC,
 * whose bodies share those sites word for word; the motion block is this span's
 * own and is written from its original bytes.
 *
 * Representation notes: spelling the motion block as byte-offset FIELD_REF
 * statements in the order the stores appear in the image (0x10 decay, 0x0C
 * decay, byte decay, then the two integrations) measured 48/66 instructions
 * over 268 bytes with the first difference at +0x64, because the allocator
 * handed the 0x10 decay the register the 0x0C decay takes here. One word-typed
 * view of the work pointer with the 0x0C decay first, the 0x10 decay second, the
 * two integrations next and the byte decay last measured 66/66 over 264 bytes
 * with no difference: the order above is the one the allocator and scheduler
 * agree with, and the stores it emits still land in the image's order.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimRecordEveryFourthTickRunPassesDrawQuadsIntegrateMotionReloadCountdownAndAdvanceStateWorld02Area09813_801F3BD4(
    void) {
  u8 tick;
  u8* work;
  u32* words;

  tick = scratchWorkPointerAt1f800044[6]++;
  if ((tick & 3) == 0) {
    claimFreeWorkRecordSeedPositionAndFadeWorld02Area09813_801F526C();
  }
  func_801F5308();
  func_801F5940();
  func_801F48E8(scratchWorkPointerAt1f800044 + 0x18,
                scratchWorkPointerAt1f800044 + 0x34, 0x40);
  work = scratchWorkPointerAt1f800044;
  words = (u32*)work;
  words[3] -= 0x800;
  words[4] -= 0x200;
  words[6] += words[3];
  words[7] += words[4];
  work[3] -= 4;
  if (--scratchWorkPointerAt1f800044[9] == 0) {
    scratchWorkPointerAt1f800044[9] = 0x80;
    scratchWorkPointerAt1f800044[1]++;
  }
}
