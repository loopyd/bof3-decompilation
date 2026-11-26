#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* workRecordCursor1F800044;

/* Overlay-resident per-entry helper at 0x801F30D0, still modelled as an asm
 * row in this target: it copies the work object's position triple at
 * +0x34/+0x38/+0x3C into the 0x2C-byte effect entry it is handed, seeds that
 * entry's corner offsets from the shared angular pair with a random rotation,
 * arms its scale and shade halfwords, and returns nothing. It takes the one
 * pointer argument this payload passes in $a0. */
void func_801F30D0(u8* entry);

/* @source 0x801F2C60
 * @behavior Entry 0 of the seven-word overlay step handler run D_801F3ED8,
 *           which dispatchByte1Table3ed8 indexes
 *           with the unsigned work byte at +0x1 of the work record published at
 *           the scratchpad pointer cell 0x1F800044. It takes no arguments and
 *           returns nothing. It walks the 0x2C-byte-stride effect table at
 *           0x800E4800 with an unsigned byte counter over that table's 0x20
 *           entries, calling the per-entry seed helper 0x801F30D0 once for each
 *           entry and advancing the entry cursor by 0x2C per iteration, so the
 *           whole table is re-seeded from the work position. After the walk it
 *           zeroes the work record's byte at +0x9 and increments the state byte
 *           at +0x1, which leaves the next frame of that dispatch on handler 1
 *           of the same run, the draw pass 0x801F2CDC.
 *
 * Evidence from the original bytes at 0x801F2C60: the 0x20-byte frame with $ra
 * at 0x18, $s1 at 0x14 and $s0 at 0x10; the lui/ori pair that materialises
 * 0x800E4800 into $s1 before the loop, $s0 zeroed as the counter, the loop's
 * single jal at 0x801F2C7C to 0x801F30D0 with addu $a0,$s1,$zero in its delay
 * slot, then addiu $s0,0x1 with andi $v0,$s0,0xFF and sltiu $v0,$v0,0x20 as the
 * bottom test whose branch delay slot carries addiu $s1,$s1,0x2C - an unsigned
 * byte counter, not a plain word one. The tail is two independent lui/lw pairs
 * on 0x1F800044, the first storing $zero at +0x9 and the second reading +0x1,
 * adding one and storing it back, so the cell is re-read between the two
 * statements rather than carried in a register across them. The call helper
 * returns through its own $ra at 0x5E0 with no $v0 set, so no result is
 * consumed here.
 *
 * Name and shape evidence: the exact lifts
 * seedEffectEntryFromWorkPositionWithRandomCornerRotationAndScaleWorld04Area18813_801F4394
 * and
 * seedEffectEntryFromWorkPositionWithRandomCornerRotationAndScaleWorld04Area17213_801F3E90
 * carry this same per-entry seed body and name its role, and the exact lift
 * retreatWorkExtentAndAdvanceOriginZDrawThenOnCountdownExpiryArm20ReseedEffectEntriesQueueCue215AndAdvanceWorkByte1World04Area18813_801F3484
 * runs this same bottom-tested 0x2C-stride seed walk over the effect block at
 * 0x800E4800 and the same clear-then-advance pair on the work record, so it
 * supplied the statement order, the offset spelling and the loop form only;
 * this span's own literals - base 0x800E4800, stride 0x2C, count 0x20 and the
 * cleared offset 9 - were written from its bytes.
 *
 * Clone evidence: a byte scan of every payload under out/binaries finds this
 * 0x7C-byte span at no other offset, so no clone applies. The nearest sibling
 * sits at offset 0x5C of emi/world04/area175/13, whose own row is still asm and
 * which therefore has no source to clone: the two 0x7C-byte spans differ in
 * exactly one word, its jal to 0x801F30CC against the jal to 0x801F30D0 here,
 * and those two addresses are the two spans' own starts shifted by the 4-byte
 * delta between them, so it is shape evidence only.
 *
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateEffectEntriesPosition(void) {
  u8* entry;
  u8 index;

  entry = PSX_PTR(u8, 0x800E4800u);
  index = 0u;
  do {
    func_801F30D0(entry);
    entry += 0x2Cu;
    index += 1u;
  } while (index < 0x20u);
  workRecordCursor1F800044[9] = 0;
  workRecordCursor1F800044[1]++;
}
