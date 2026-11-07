#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui 0x1F80 plus a load at displacement 0x44 for each of the four store
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
void func_801F44B4(void);
void func_801F3E7C(void);

/* @source 0x801F3598
 * @behavior Overlay progress step handler. It reads the byte at offset 0x06 of
 * the work record published at the scratchpad pointer cell 0x1F800044 and
 * stores that byte plus one back, so the byte counts the frames this handler
 * runs; whenever the pre-increment value's low two bits are clear - every
 * fourth frame - it calls the work-record spawn 0x801F3DE0, which claims the
 * first free entry of the 0x18-byte-stride record table at 0x800E4800 and
 * seeds it from this work record. It then runs this overlay's two per-frame
 * passes 0x801F44B4 and 0x801F3E7C in that order and, re-reading the record
 * through the cell, decrements the countdown byte at offset 0x09. While that
 * byte stays non-zero the handler returns. On the frame it reaches zero the
 * handler reloads the cell once more and rewinds the record: the two step
 * words at 0x0C and 0x10 take -0x1000 and -0x400, the three words at 0x18,
 * 0x1C and 0x20 take the origin triple the record keeps at 0x64, 0x68 and
 * 0x6C, and the state byte at offset 0x01 advances by one, which moves the
 * overlay to the next progress step. It takes no arguments, returns nothing
 * and its 0x18-byte frame only holds $ra at 0x10 across the three calls.
 *
 * Evidence: payload asm at 0x801F3598 - lui 0x1F80 plus lw 0x44 before the
 * frame is created, addiu $sp,-0x18, sw $ra,0x10($sp), lbu $v1,0x6($v0), nop,
 * addiu $a0,$v1,1, andi $v1,$v1,3, bnez to the join with sb $a0,0x6($v0) in
 * its delay slot, jal 0x801F3DE0 only on the fall-through, then jal 0x801F44B4
 * and jal 0x801F3E7C with nop delay slots, a fresh lui/lw pair of the same
 * cell, lbu $v0,0x9($v1), addiu $v0,$v0,-1, sb $v0,0x9($v1), andi $v0,0xFF,
 * bnez to the shared epilogue, a third lui/lw pair, lw 0x64/lw 0x68, sw
 * $v1,0xC with $v1 = 0xFFFFF000 already staged in the delay slot, sw $v1,0x10
 * with $v1 = 0xFFFFFC00, lbu 0x1 plus addiu 1, lw 0x6C, then the three stores
 * to 0x18, 0x1C and 0x20 and sb to 0x1.
 *
 * Field roles: the 0x18/0x1C pair accumulates the 0x0C/0x10 step words in the
 * sibling handler 0x801F36C4 of this same payload, which subtracts 0x800 and
 * 0x200 from them and adds each into 0x18 and 0x1C, so 0x0C/0x10 are the
 * per-frame steps and 0x18/0x1C/0x20 the triple they move. The 0x64/0x68/0x6C
 * triple is the record's origin: this payload's exact
 * claimFreeWorkRecordSeedPositionAndFadeWorld04Area18813_801F3DE0 seeds a
 * spawned record's position from the nearby 0x34/0x38/0x3C triple, and the
 * exact scenario lift func_801F85A8 reads the 0x64/0x68/0x6C words as one
 * origin vector.
 *
 * Shape evidence: the fourth-tick claim, the two passes and the countdown
 * byte this payload's exact
 * claimWorkRecordEveryFourthTickThenArmWorkByte9AndAdvanceWorkByte1World04Area18813_801F364C
 * records are the same prefix here, written with the same statement order and
 * the same per-store-group reload of the cell. The countdown-expiry tail
 * follows the exact sibling advanceStepDrawTileUntilCountdownThenSeedStepsAndArmEffectWorld04Area17213_801F342C,
 * which decrements byte 0x09 the same way, rewinds with two word stores at
 * 0x0C/0x10 and advances byte 0x01 from a re-read of the cell; this function's
 * own constants, field offsets and reload points come from the original bytes.
 *
 * Name evidence: the only in-image reference to this address is the word at
 * 0x801F4E14, inside the seven-word code-pointer run at 0x801F4E0C that also
 * holds 0x801F3400, 0x801F3484, 0x801F364C, 0x801F36C4, 0x801F37CC and
 * 0x801F38C0; no in-payload code loads that run or calls this address, so the
 * run names this function's position among the overlay's progress handlers and
 * the caller stays outside the payload.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkWords18To20FromOriginAndSetStepWordsWhenCountdownExpiresThenAdvanceWorkByte1World04Area18813_801F3598(
    void) {
  u8* work;
  u8 tick;

  tick = D_1F800044[6]++;
  if ((tick & 3) == 0) {
    claimFreeWorkRecordSeedPositionAndFadeWorld04Area18813_801F3DE0();
  }
  func_801F44B4();
  func_801F3E7C();
  if (--D_1F800044[9] == 0) {
    work = D_1F800044;
    *(s32*)(work + 0xC) = -0x1000;
    *(s32*)(work + 0x10) = -0x400;
    *(u32*)(work + 0x18) = *(u32*)(work + 0x64);
    *(u32*)(work + 0x1C) = *(u32*)(work + 0x68);
    *(u32*)(work + 0x20) = *(u32*)(work + 0x6C);
    work[1]++;
  }
}
