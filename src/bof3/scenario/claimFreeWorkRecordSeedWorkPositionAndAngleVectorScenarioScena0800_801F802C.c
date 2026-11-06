#include "bof3/bof3.h"

extern u8* scenarioScena0800Work; /* @source 0x1F800044 @kind unknown */
u8* findFreeWorkRecordScenarioScena0800_801F7DAC(void);
s32 func_801782FC(s32 arg0); /* @source 0x801782FC */
s32 func_801783C8(s32 arg0); /* @source 0x801783C8 */

/* @source 0x801F802C
 * @behavior Claims a free work record and seeds it from the scratch work
 * object: the overlay-local scan at 0x801F7DAC returns the first of the 0x40
 * entries of the 0x28-byte-stride record table at 0x800E4800 whose lead byte
 * reads zero, and when every entry is claimed the function returns without
 * writing anything. Otherwise it stores 1 in the record's claim byte at offset
 * 0, clears its byte at offset 2, and copies the scratch work object published
 * at pointer slot 0x1F800044 into the record: word 0x04 takes the work word
 * 0x34, word 0x08 the work word 0x38 and word 0x0C the work word 0x3C
 * arithmetic-shifted right by 8. The work object's word at 0x0C then selects a
 * direction: the shared rotation helper func_801782FC (sine on the
 * 0x1000-per-turn angle unit) of it is taken as an unsigned value, shifted
 * right by 4, masked to 16 bits and biased by 0x800 when the reloaded work
 * object's byte at offset 6 reads 7, else by 0x400. That biased angle is handed
 * to the cosine companion func_801783C8 and to func_801782FC, and three times
 * each result lands in the record's words at 0x14 and 0x18 while word 0x1C is
 * cleared, so the record's second triple is the angle's scaled direction vector
 * with a zero third component. The scratch work pointer is read once for the
 * position triple and reloaded before the byte-6 test; takes no arguments,
 * returns nothing, and its 0x20-byte frame keeps $ra plus $s1 for the record
 * and $s0 for the biased angle, both of which survive the three calls.
 *
 * Name evidence: this target's exact lift findFreeWorkRecordScenarioScena0800_801F7DAC
 * records itself as the work-record claim scan over the 0x40 entries of the
 * 0x28-byte-stride table at 0x800E4800 and records its only caller - this
 * address at 0x801F8038 - as testing the returned pointer for zero and then
 * writing 1 to byte 0, 0 to byte 2 and filling the words at 0x04/0x08/0x0C from
 * the scratch work object, which is the head of this body. The record field
 * roles follow the same target's seedClaimedWorkRecordFromWorkObjectScenarioScena0800_801F8B50,
 * which calls the same 0x34/0x38/0x3C cell triple the position triple and byte 0
 * the claim flag. The only in-image reference to this address is jal 0x801F802C
 * at 0x801F6FDC inside
 * tickCountdownByte9Advance12BitWorkWord0CAndStateScenarioScena0800_801F6FC4,
 * which passes no arguments and discards the result. func_801782FC and
 * func_801783C8 are the concurrently loaded etc/game overlay's 12-bit rotation
 * pair that this overlay's own
 * shiftTableEntriesAndInsertHeadValueScenarioScena0800_801F81F4 and
 * func_801F8360 name as the shared sine/cosine helpers; the record's seeded
 * vector roles beyond that angle derivation stay described by offset.
 *
 * Matching note: the two bias arms mask and then bias the angle as two
 * in-place statements (`angle = angle & 0xFFFF; angle += bias;`). Writing the
 * same truncation as one combined expression let the compiler share a single
 * mask on the branch and fill the branch delay slot, which is two instructions
 * short of the original; the split form keeps one in-place mask inside each
 * arm, so the original's unfilled delay slot and its in-place `andi`/`addiu`
 * pair are reproduced with identical memory events.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimFreeWorkRecordSeedWorkPositionAndAngleVectorScenarioScena0800_801F802C(void) {
  u8* record;
  u8* work;
  u32 angle;

  record = findFreeWorkRecordScenarioScena0800_801F7DAC();
  if (record != 0) {
    record[0] = 1;
    record[2] = 0;
    work = scenarioScena0800Work;
    *(u32*)(record + 0x04) = *(u32*)(work + 0x34);
    *(u32*)(record + 0x08) = *(u32*)(work + 0x38);
    *(s32*)(record + 0x0C) = *(s32*)(work + 0x3C) >> 8;
    angle = (u32)func_801782FC(*(s32*)(work + 0x0C)) >> 4;
    work = scenarioScena0800Work;
    if (work[6] == 7) {
      angle = angle & 0xFFFF;
      angle += 0x800;
    } else {
      angle = angle & 0xFFFF;
      angle += 0x400;
    }
    *(u32*)(record + 0x14) = func_801783C8(angle) * 3;
    *(u32*)(record + 0x18) = func_801782FC(angle) * 3;
    *(u32*)(record + 0x1C) = 0;
  }
}
