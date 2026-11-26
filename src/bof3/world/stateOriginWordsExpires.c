#include "bof3/bof3.h"

/* Published scratch work object: the original materialises this cell with a
 * lui 0x1F80 plus a load at displacement 0x44 for each of the four store
 * groups below, so the cell is read through this target's map name rather than
 * the constant-address accessor, whose shared base would stay live across the
 * calls in a saved register and widen the frame. */
extern u8* scratchWorkPointerAt1; /* @source 0x1F800044 @kind data */

void stateClaimFreeRecord(void);

/* Per-frame passes this overlay's progress handlers run, still anchored to
 * their addresses. 0x801F5940 is the draw pass over the 0x800E48C0 table.
 * 0x801F5308 is the record pass over the 0x800E4800 table. This handler runs
 * the draw pass first and the record tick pass second, the order its own two
 * jal words give. */
void func_801F5940(void);
void func_801F5308(void);

/* @source 0x801F3AA8
 * @behavior Overlay progress step handler, entry 4 of the nine-word handler
 *           row based at 0x801F5F88. It reads the byte at offset 0x06 of the
 *           work record published at the scratchpad pointer cell 0x1F800044
 *           and stores that byte plus one back, so the byte counts the frames
 *           this handler runs; whenever the pre-increment value has its low
 *           two bits clear - every fourth frame - it calls the work-record
 *           spawn 0x801F526C, which claims the first free entry of the
 *           0x18-byte-stride record table at 0x800E4800 and seeds it from this
 *           work record. It then runs two per-frame passes in the payload
 *           order, the 0x800E48C0 draw pass 0x801F5940 and the 0x800E4800
 *           record tick pass 0x801F5308, and, re-reading the record through
 *           the cell, decrements the countdown byte at offset 0x09. While that
 *           byte stays non-zero the handler returns. On the frame it reaches
 *           zero the handler reloads the cell once more and rewinds the
 *           record: the two step words at 0x0C and 0x10 take -0x1000 and
 *           -0x400, the three words at 0x18, 0x1C and 0x20 take the origin
 *           triple the record keeps at 0x64, 0x68 and 0x6C, and the state byte
 *           at offset 0x01 advances by one, which hands the record to entry 5
 *           of the row, 0x801F3B5C. It takes no arguments, returns nothing and
 *           its 0x18-byte frame only holds $ra at 0x10 across the three calls.
 *
 * Evidence: payload asm at 0x801F3AA8 - lui 0x1F80 plus lw 0x44 before the
 * frame is created, addiu $sp,-0x18, sw $ra,0x10($sp), lbu $v1,0x6($v0), nop,
 * addiu $a0,$v1,1, andi $v1,$v1,3, bnez to the join with sb $a0,0x6($v0) in
 * its delay slot, jal 0x801F526C only on the fall-through, then jal 0x801F5940
 * and jal 0x801F5308 with nop delay slots, a fresh lui/lw pair of the same
 * cell, lbu $v0,0x9($v1), nop, addiu $v0,$v0,-1, sb $v0,0x9($v1), andi
 * $v0,0xFF, bnez to the shared epilogue with addiu $v1,$zero,-0x1000 in its
 * delay slot, a third lui/lw pair, lw 0x64/lw 0x68, sw $v1,0xC, sw $v1,0x10
 * with $v1 = 0xFFFFFC00, lbu 0x1 plus addiu 1, lw 0x6C, then the three stores
 * to 0x18, 0x1C and 0x20 and sb to 0x1.
 *
 * Field roles: the 0x18/0x1C pair accumulates the 0x0C/0x10 step words in the
 * neighbouring handler 0x801F3BD4 of this same payload, which subtracts 0x800
 * and 0x200 from them and adds each into 0x18 and 0x1C, so 0x0C/0x10 are the
 * per-frame steps and 0x18/0x1C/0x20 the triple they move. The 0x64/0x68/0x6C
 * triple is the record origin: this payload's exact
 * stateClaimFreeRecord seeds a
 * spawned record's position from the nearby 0x34/0x38/0x3C triple.
 *
 * Shape evidence: this 180-byte span, payload offsets 0xEA8-0xF5B, carries the
 * same fourth-tick claim, the same two passes and the same countdown byte as
 * the neighbouring exact
 * claimEveryFourthPasses,
 * with the same statement order and the same per-store-group reload of the
 * cell. The full body is byte-identical to the 180-byte span at payload offset
 * 0x998 of emi/world04/area188/13, whose lift recorded an exact byte match as
 * seedWorkWords18To20FromOriginAndSetStepWordsWhenCountdownExpiresThenAdvanceWorkByte1World04Area18813_801F3598,
 * except its three jal own-immediates - 0x0C07CF78, 0x0C07D12D and 0x0C07CF9F
 * there, whose targets 0x801F3DE0, 0x801F44B4 and 0x801F3E7C are that overlay's
 * spawn, draw pass and record tick pass - against 0x0C07D49B, 0x0C07D650 and
 * 0x0C07D4C2 here, whose targets 0x801F526C, 0x801F5940 and 0x801F5308 are
 * this overlay's spawn and the same two passes in the same order. Every other
 * word is equal, so that twin supplied the statement order, the operand forms
 * and the offset spelling, and this function's three call targets and its cell
 * binding are written from this span's own bytes and this target's own map.
 *
 * Name evidence: the payload word at 0x801F5F98 is the fifth entry, index 4,
 * of the nine-word handler row based at 0x801F5F88 (0x801F3814, 0x801F3878,
 * 0x801F3910, 0x801F3994, 0x801F3AA8, 0x801F3B5C, 0x801F3BD4, 0x801F3CDC,
 * 0x801F3D98; the next dispatcher row starts at 0x801F5FAC). The exact
 * dispatcher dispatchStateByte1Table5 scales the
 * work record state byte at +0x01 by four, loads that word from the row and
 * jalrs it, so the row fixes this function as the state-4 progress handler and
 * its caller stays outside the payload. Entry 5 of the same row, 0x801F3B5C,
 * is where this handler hands the record through the +0x01 advance.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateOriginWordsExpires(
    void) {
  u8* work;
  u8 tick;

  tick = scratchWorkPointerAt1[6]++;
  if ((tick & 3) == 0) {
    stateClaimFreeRecord();
  }
  func_801F5940();
  func_801F5308();
  if (--scratchWorkPointerAt1[9] == 0) {
    work = scratchWorkPointerAt1;
    *(s32*)(work + 0xC) = -0x1000;
    *(s32*)(work + 0x10) = -0x400;
    *(u32*)(work + 0x18) = *(u32*)(work + 0x64);
    *(u32*)(work + 0x1C) = *(u32*)(work + 0x68);
    *(u32*)(work + 0x20) = *(u32*)(work + 0x6C);
    work[1]++;
  }
}
