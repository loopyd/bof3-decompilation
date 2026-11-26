#include "bof3/bof3.h"

u8* findFreeWorkRecord(void);
void claimWorkRecordAndSeedPositionStepSizeAndHeadingFromWorkObjectWorld04Area17413_801F3378(
    u8* record);
void func_801F2EC4(void);

/* Published scratchpad work-record pointer cell. This body re-materialises the
 * cell for each access group - once for the substate-counter mask, once for the
 * counter advance, once for the mode byte and once for the state advance - so
 * every statement spells the cell rather than a held local.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* @source 0x80147A8C @kind data
 * Resident position triplet base in main RAM; overlay sources copy these three
 * words into their work objects as a position pair plus a third word, and this
 * body compares the first of them against a fixed world coordinate.
 */
extern s32 D_80147A8C;

/* @source 0x801F2CA8
 * @behavior Entry 1 of this overlay's work-byte-1 state machine: entry 1 of the
 *           eight-word handler run based at 0x801F4FAC, which this target's
 *           exact dispatcher dispatchStateByte1Table4
 *           indexes with the work record's state byte at +1. That base sits
 *           inside the payload textbin blob, so the word at 0x801F4FAC names
 *           this span and no jal in the payload reaches it. Entry 0 at
 *           0x801F2C48 clears that substate counter and advances the state byte
 *           so the next frame dispatches here.
 *           While the low three bits of the work record's substate counter byte
 *           at +9 read zero - every eighth tick of that counter - it runs the
 *           claim scan at 0x801F2E88 over the 0x20 records of the 0x2C-byte-
 *           stride table at 0x800E4800 and, when that scan answers a record,
 *           passes it to the seeder at 0x801F3378, which claims the record and
 *           fills its position, step, size, countdown and heading from the
 *           scratch work object. It then advances the counter byte at +9 by one
 *           and, while the work record's mode byte at +6 reads nonzero, runs the
 *           record stepper at 0x801F2EC4 that walks the same table. Finally,
 *           when the resident position word D_80147A8C equals the fixed
 *           coordinate 0x3C8000, it advances the state byte at +1 by one, so the
 *           next frame dispatches entry 2 at 0x801F2D68. Takes no arguments and
 *           returns nothing; the 0x18-byte frame only keeps $ra at 0x10 across
 *           the three calls, and the comparison constant is materialised once
 *           in the mode-byte branch delay slot and again after the stepper call.
 *           The record fields stay addressed by their anchored offsets.
 *
 * Name evidence: the spawn arm comes from this body's own claim scan and seeder
 * call pair and from the mask on the counter byte at +9; the stepper arm from
 * the mode byte at +6 and the call at 0x801F2EC4. The position test is this
 * span's own word: it loads the shared word at 0x80147A8C and compares it with
 * 0x3C8000, a world coordinate of the same form the scenario overlays pass to
 * their cue requests.
 * @status exact
 * @match 100.00
 * @residual none
 */
void spawnWorkRecordEveryEighthTickOfByte9StepWorkRecordsWhileByte6SetThenAdvanceWorkByte1WhenPositionEquals3c8000World04Area17413_801F2CA8(
    void) {
  u8* record;

  if ((D_1F800044[9] & 7) == 0) {
    record = findFreeWorkRecord();
    if (record != 0) {
      claimWorkRecordAndSeedPositionStepSizeAndHeadingFromWorkObjectWorld04Area17413_801F3378(
          record);
    }
  }
  D_1F800044[9] += 1;
  if (D_1F800044[6] != 0) {
    func_801F2EC4();
  }
  if (D_80147A8C == 0x3C8000) {
    D_1F800044[1] += 1;
  }
}
