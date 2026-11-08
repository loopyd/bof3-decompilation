#include "bof3/bof3.h"

/* Published scratch work object. The original materialises this cell with its
 * own lui 0x1F80 plus a load at displacement 0x44 for each store group, before
 * the frame is created for the first group and again after the front-end call
 * for the last one. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* Shared front-end effect-busy halfword at 0x80143C40, the same word the
 * sibling state handlers in this overlay test. */
extern u16 D_80143C40; /* @source 0x80143C40 @kind unknown */

/* Shared effect-bank byte at 0x8014832E, bound by the shared map. */
extern u8 D_8014832E; /* @source 0x8014832E @kind unknown */

/* The three per-frame passes this handler runs, described here by address.
 * 0x801F38E8 walks the 0x18-byte-stride record table at 0x800E4800 for the
 * first free entry, marks it active and seeds it with byte 2 = 0x20 before
 * reloading the work record. 0x801F3FAC is the tile draw pass: it asks for a
 * 0x380 tpage, sets the draw mode and builds quads through g_PrimCursor.
 * 0x801F3984 is the record pass over the same 0x800E4800 table. */
void func_801F38E8(void);
void drawEffectEntriesThenAdvanceAngle24By10World04Area17213_801F3FAC(void);
void stepActiveWorkRecordCountdownAndHalfword14ThenDrawActiveRecordsWorld04Area17213_801F3984(
    void);

/* @source 0x801F32E8
 * @behavior Overlay progress step handler, entry 4 of the state byte 1
 *           dispatch table at 0x801F5090. It reads byte 0x06 of the work
 *           record published at the scratchpad pointer cell 0x1F800044 and
 *           stores that byte plus one back, so the byte counts the frames this
 *           handler runs; whenever the pre-increment value's low two bits are
 *           clear - every fourth frame - it calls the record claim 0x801F38E8.
 *           It then runs the overlay tile draw pass 0x801F3FAC and the record
 *           pass 0x801F3984 in that order and, while the shared effect-busy
 *           halfword 0x80143C40 reads zero, sets front-end mode 9 through
 *           0x8014ECAC, clears the shared effect-bank byte 0x8014832E and,
 *           re-reading the work record through the cell, advances state byte
 *           0x01 by one so the dispatcher moves to the next step. A busy
 *           effect leaves the mode, the shared byte and the state byte
 *           untouched. Takes no arguments and returns nothing; its 0x18-byte
 *           frame only keeps $ra at 0x10 across the three calls.
 *
 * Evidence: payload asm at 0x801F32E8 - lui 0x1F80 plus lw 0x44 before the
 * frame is created, addiu $sp,-0x18, sw $ra,0x10($sp), lbu $v1,0x6($v0), nop,
 * addiu $a0,$v1,1, andi $v1,$v1,3, bnez to the join with sb $a0,0x6($v0) in
 * its delay slot, jal 0x801F38E8 only on the fall-through, then jal 0x801F3FAC
 * and jal 0x801F3984 with nop delay slots, lhu $v0 of 0x80143C40, nop, bnez
 * to the epilogue, jal 0x8014ECAC with addiu $a0,$zero,0x9 in its delay slot,
 * a fresh lui/lw pair of the cell, lui $at of 0x8014832E with sb $zero,
 * lbu $v0,0x1($v1), nop, addiu $v0,$v0,1, sb $v0,0x1($v1), then lw
 * $ra,0x10($sp), addiu $sp,0x18 and jr $ra with a nop delay slot.
 *
 * Name evidence: no jal in the payload targets this address; its only in-image
 * reference is the word at 0x801F50A0, entry 4 of the code-pointer run at
 * 0x801F5090. That run is indexed by work record byte 0x01 through the
 * dispatcher dispatchStateByte1Table5090World04Area17213_801F2FF0, and its
 * entry 5 is the exact sibling setSubstate14AndClearWorkFlagsWhenEffectIdle
 * World04Area17213_801F3374, which reads the same effect-busy halfword. The
 * cadence and statement order follow the near-identical progress handler
 * claimWorkRecordEveryFourthTickThenArmWorkByte9AndAdvanceWorkByte1World04
 * Area18813_801F364C, whose body differs in the call targets, the effect gate
 * and its stored literal.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimWorkRecordEveryFourthTickThenDrawSlotsAndAdvanceStateWhenEffectIdleWorld04Area17213_801F32E8(
    void) {
  u8 tick;

  tick = D_1F800044[6]++;
  if ((tick & 3) == 0) {
    func_801F38E8();
  }
  drawEffectEntriesThenAdvanceAngle24By10World04Area17213_801F3FAC();
  stepActiveWorkRecordCountdownAndHalfword14ThenDrawActiveRecordsWorld04Area17213_801F3984();
  if (D_80143C40 == 0) {
    func_8014ECAC(9);
    D_8014832E = 0;
    D_1F800044[1]++;
  }
}
