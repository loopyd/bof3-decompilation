#include "bof3/bof3.h"

/* @source 0x801AFE18
 * Shared projection setup consuming the stack scratch area the caller passes in
 * a0; no other argument register is live at the call site.
 */
extern void func_801AFE18(void* arg0);

/* @source 0x8015477C
 * Shared helper over the projected word pair passed as (x, y); this overlay
 * publishes its result as a record word shifted left by 16.
 */
extern s16 func_8015477C(s32 arg0, s32 arg1);

/* @source 0x801F46EC
 * This target's overlay quad emitter, still an asm boundary: it builds one
 * semi-transparent textured sprite quad through the shared primitive cursor
 * (SetPolyFT4, SetSemiTrans, GetClut, GetTPage) from one work record, reading
 * the record colour byte at 0x03 into the primitive r, g and b bytes and the
 * record halfword at 0x04 as the quad size.
 */
extern void func_801F46EC(void* record);

/* Shared published work-pointer cell: the original re-materialises the cell
 * address and reloads the pointer on every iteration of the sweep below, so the
 * fixed-address cell symbol keeps that load in the loop. */
extern u8* volatile D_1F800044; /* @source 0x1F800044 @kind data */

/* One 0x28-byte record of the 0x20-entry work table at 0x800E4800, described in
 * the offsets this sweep touches. The claim helper of this target seeds the
 * record from the work object published at the scratchpad pointer cell
 * 0x1F800044: the lead byte takes the published byte 6 step stamp and is the
 * claim flag, the byte at 0x03 takes 0x80 and the halfword at 0x04 takes 0x80.
 * This target's overlay quad emitter reads the byte at 0x03 into the primitive
 * r, g and b bytes and the halfword at 0x04 as the quad size, whose half extent
 * is that halfword shifted right by one, so those two roles are the emitter's.
 * The word at 0x08 takes the published word 0x34 and the word at 0x0C the
 * published word 0x38 - the position pair this overlay republishes through the
 * same offsets - and the words at 0x18 and 0x1C are the pair this sweep adds to
 * them, with 0x18 seeded to 0x1800. The word at 0x10 holds the projected pair
 * scaled left by 16 and the remaining bytes are untouched here.
 */
typedef struct Area13413ActiveWorkRecord {
  u8 stamp_00;
  u8 state_01;
  u8 counter_02;
  u8 color_03;
  u16 size_04;
  u8 pad_06[2];
  s32 position_08;
  s32 position_0c;
  s32 scaled_10;
  u8 pad_14[4];
  s32 velocity_18;
  s32 velocity_1c;
  u8 pad_20[8];
} Area13413ActiveWorkRecord;

/* @source 0x801F49A0
 * @behavior Per-frame sweep of the 0x20 records of the 0x28-byte-stride work
 * table at 0x800E4800: it prepares the shared projection scratch area through
 * the overlay setup at 0x801AFE18, then visits every record whose lead byte at
 * offset 0 equals the byte 6 stamp of the work object published at the
 * scratchpad pointer cell 0x1F800044 - the stamp the claim helper of this
 * target copies there, and the claim flag itself. When at least one record
 * carries that stamp the sweep leaves 1, otherwise 0. For each stamped record
 * it advances a four-stage ramp keyed by the state byte at offset 0x01, each
 * stage counting the byte at 0x02 and testing it against its own limit: stage 0
 * raises the size halfword at 0x04 by 4 until the counter reads 8, stage 1
 * lowers the colour byte at 0x03 by 8 until the counter reads 0x10, stages 2
 * and 3 lower the size halfword by 0x10 until the counter reads 0x18 and 0x20,
 * each of the first three stages advancing the state byte by one at its limit
 * and stage 3 clearing the lead byte to release the record. Every stamped
 * record, whatever its state, then adds the velocity word at 0x18 to the
 * position word at 0x08, raises the size halfword by 8, adds the velocity word
 * at 0x1C to the position word at 0x0C, stores this overlay's projection of the
 * two position words scaled left by 16 in the word at 0x10 and hands the record
 * to the overlay quad emitter at 0x801F46EC. Takes no arguments; its 0x58-byte
 * frame holds the 0x30-byte scratch area the setup fills at $sp+0x10 plus the
 * saved $s0-$s3 and $ra, and the loop counter is an unsigned byte compared
 * below 0x20.
 *
 * Name evidence: the only in-image references to this address are the two tail
 * calls of this target's exact
 * claimWorkRecordEveryFourthCountdownTickThenAdvanceWorkByte1World03Area13413_801F4288
 * and clearWorkFlagsWhenNoCallbackRanWorld03Area13413_801F4310, both of which
 * record it as the walk over the 0x20 entries of the 0x28-byte-stride table at
 * 0x800E4800 whose result is 1 once at least one entry ran. The stamp test is
 * the claim flag the exact
 * claimFreeWorkRecordSeedWorkPositionWorld03Area13413_801F48B0 of this target
 * seeds from published work byte 6, and the tail emitter 0x801F46EC is the
 * overlay quad emitter the same table feeds, so the role is the stamped-record
 * ramp, integration and quad emission the name states. Field roles at 0x03 and
 * 0x04 come from that emitter, which reads them as the primitive colour and the
 * quad size; the remaining roles stay described by offset.
 *
 * Shape evidence: world03/area125/13 holds the same sweep shape at 0x801F33B0,
 * byte-identical for its first 0x38 bytes and in its whole integration tail -
 * the 24-byte run from 0x1EEC here is the same 24 bytes as that sibling's
 * 0x884 - but it filters on a nonzero lead byte, runs two stages over the
 * colour byte and sweeps 0x10 records, so 69 of its 85 comparable words differ
 * and it is shape evidence, never clone eligibility. Its exact source
 * src/bof3/world/rampIntensityIntegrateAndRenderActiveWorkRecordsWorld03Area12513_801F33B0.c
 * supplies the statement order of the integration tail, the 0x30-byte scratch
 * declaration, the u8 loop counter and accumulator spellings, the u8 switch
 * variable, the stage statement order and the single struct pointer; this
 * function's own stamp test, stages, limits and callees come from its original
 * bytes. No payload among the 396 binaries under out/binaries carries a
 * byte-identical copy of this 0x1CC-byte span, and no other payload carries its
 * 0x18 and 0x20 stage limits, so the data, carve and clone levers are all
 * declined on this range.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 advanceStampedWorkRecordRampsIntegrateAndEmitQuadWorld03Area13413_801F49A0(
    void) {
  Area13413ActiveWorkRecord* record;
  u8 scratch[0x30];
  u8 i;
  u8 handled;
  u8 count;
  s32 y;

  func_801AFE18(scratch);

  handled = 0;
  record = PSX_PTR(Area13413ActiveWorkRecord, 0x800E4800u);
  i = 0;
  do {
    if (record->stamp_00 == D_1F800044[6]) {
      handled = 1;
      switch (record->state_01) {
      case 0:
        record->size_04 += 4;
        count = record->counter_02 += 1;
        if (count == 8) {
          record->state_01 += 1;
        }
        break;
      case 1:
        record->color_03 -= 8;
        count = record->counter_02 += 1;
        if (count == 0x10) {
          record->state_01 += 1;
        }
        break;
      case 2:
        record->size_04 -= 0x10;
        count = record->counter_02 += 1;
        if (count == 0x18) {
          record->state_01 += 1;
        }
        break;
      case 3:
        record->size_04 -= 0x10;
        count = record->counter_02 += 1;
        if (count == 0x20) {
          record->stamp_00 = 0;
        }
        break;
      }
      record->position_08 += record->velocity_18;
      record->size_04 += 8;
      y = record->position_0c + record->velocity_1c;
      record->position_0c = y;
      record->scaled_10 = func_8015477C(record->position_08, y) << 16;
      func_801F46EC(record);
    }
    record++;
    i++;
  } while (i < 0x20);

  return handled;
}
