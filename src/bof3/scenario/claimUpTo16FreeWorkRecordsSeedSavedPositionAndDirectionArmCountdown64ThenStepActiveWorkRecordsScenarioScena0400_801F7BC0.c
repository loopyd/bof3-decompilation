#include "bof3/bof3.h"

/* Shared saved work triplet: the three consecutive 32-bit words at
 * 0x80146954, 0x80146958 and 0x8014695C, which this body reads through one
 * base pointer because the overlay's seeding handlers copy them together. */
extern s32 D_80146954;

/* Shared phase samplers at 0x801782FC and 0x801783C8: the sine and the cosine
 * of a 12-bit phase. */
s32 func_801782FC(s32 phase);
s32 func_801783C8(s32 phase);

u8* findFreeWorkRecordByte25ScenarioScena0400_801F7FF8(void);
void func_801F7EC0(void);

/* @source 0x801F7BC0
 * @behavior Entry 2 of this overlay's eight-word work-object handler run at
 *           0x801FA1E4 ({0x801F7AC8, 0x801F7B00, 0x801F7BC0, 0x801F8124,
 *           0x801F9D48, 0x801F9DB4, 0x801F9DAC, 0x801FA010}), reached only
 *           through the indirect call in
 *           dispatchWorkObjectHandlerScenarioScena0400_801F7A84 (0x801F7A84),
 *           which indexes that run with byte 0x1 of the scratch work object
 *           published at pointer slot 0x1F800044.
 *           It walks sixteen index values and, for each one, claims a free
 *           record through the work-table scan
 *           findFreeWorkRecordByte25ScenarioScena0400_801F7FF8 (jal at
 *           0x801F7BEC). An index whose scan answers zero seeds nothing and
 *           the walk moves on, so the body claims as many records as it can
 *           find up to sixteen. Each claimed record takes the shared saved
 *           triplet at +0x00, +0x04 and +0x08, the last arithmetic-shifted
 *           right by 8, then the sine sampler func_801782FC (jal at 0x801F7C24)
 *           and its cosine companion func_801783C8 (jal at 0x801F7C34), both
 *           called with the index scaled by 0x100 on their 12-bit phase unit
 *           and both answers arithmetic-shifted right by 2, which land at
 *           +0x10 and +0x14. Since 0x1000 is one full turn, the sixteen
 *           indices are evenly spaced directions around the circle. The
 *           record's +0x18 word is cleared, +0x20 takes 0x800, and the claim
 *           marker 0x40 lands in bytes 0x24 and 0x25, the byte the free-record
 *           scan tests against zero.
 *           After the walk it runs the overlay's record stepper func_801F7EC0
 *           (jal at 0x801F7C6C, no arguments), re-reads the scratch work-object
 *           pointer from the cell at 0x1F800044 because the stepper may
 *           rewrite that cell, and stores 0x40 in the work object's word at
 *           +0x0C. The overlay step handler at 0x801F7CA8 decrements that word
 *           and tests it against zero, so this body arms 64 of those steps.
 *           It takes no arguments, consumes no result, and its 0x28-byte frame
 *           holds the six saved registers above the 0x10 bytes of outgoing
 *           argument space this compiler reserves for a function that calls.
 *
 * Name evidence: the only image word holding 0x801F7BC0 is offset 0x801FA1EC,
 *           word 2 of the handler run at 0x801FA1E4, so no jal names this body
 *           and the dispatch shape is the naming route rather than missing
 *           evidence. The claim-and-seed role comes from the scan call and the
 *           words each claimed record takes, the sixteen from the unsigned
 *           bound on the masked walk counter, the direction from the sine and
 *           cosine pair scaled by the index, the countdown from the 0x40 this
 *           body arms in the work object, and the stepping from the stepper
 *           call. The exact in-target handler at 0x801F7CA8 names this address
 *           as the arming writer of the countdown it ticks.
 *
 * Shape evidence: statement order, operand forms and the `*(s32*)(record + N)`
 *           spelling follow the exact in-target entry 1 at 0x801F7B00, which
 *           claims the same records from the same shared triplet and calls the
 *           same stepper, while the stepper-call tail and its `SPAD_PTR_SLOT`
 *           re-read of the work object follow the exact in-target handler at
 *           0x801F7CA8. This function's own literals, marker and offsets come
 *           from its original bytes, and the direction pair, the sixteen-step
 *           record walk and the countdown store are its own. The clone
 *           lever is declined on a masked payload scan: with every jal word
 *           masked, these 0xE8 bytes occur at no other offset of the 396
 *           payloads under out/binaries, and no unmasked hit exists either.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimUpTo16FreeWorkRecordsSeedSavedPositionAndDirectionArmCountdown64ThenStepActiveWorkRecordsScenarioScena0400_801F7BC0(
    void) {
  u8 index;
  u8* record;
  u8* work;
  s32* saved;

  index = 0u;
  saved = &D_80146954;
  do {
    record = findFreeWorkRecordByte25ScenarioScena0400_801F7FF8();
    if (record != 0) {
      *(s32*)(record + 0x00) = saved[0];
      *(s32*)(record + 0x04) = saved[1];
      *(s32*)(record + 0x08) = saved[2] >> 8;
      *(s32*)(record + 0x10) = func_801782FC(index << 8) >> 2;
      *(s32*)(record + 0x14) = func_801783C8(index << 8) >> 2;
      *(s32*)(record + 0x18) = 0;
      record[0x24] = 0x40;
      *(s32*)(record + 0x20) = 0x800;
      record[0x25] = 0x40;
    }
    index += 1u;
  } while (index < 16u);

  func_801F7EC0();
  work = SPAD_PTR_SLOT(u8, 0x44u);
  *(s32*)(work + 0x0C) = 0x40;
}
