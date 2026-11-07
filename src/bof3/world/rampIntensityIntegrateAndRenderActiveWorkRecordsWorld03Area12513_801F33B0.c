#include "bof3/bof3.h"

/* 0x801AFE18 — shared projection setup consuming the stack scratch area at
 * $sp+0x10; only a0 is live at the call site. */
void func_801AFE18(void* arg0);

/* 0x8015477C — shared helper over the word pair passed as (x, y); this overlay
 * publishes its result as a 0x10000-scaled record word. */
s16 func_8015477C(s32 arg0, s32 arg1);

/* 0x801F2FC4 — this overlay's per-record emitter: it builds one translucent
 * sprite quad from the intensity byte at +0x03, the angle halfword at +0x04 and
 * the position words at +0x08 and +0x0C. */
void func_801F2FC4(void* record);

/* One 0x28-byte record of the 0x10-entry work table at 0x800E4800. The claim
 * helper of this target seeds the position and velocity words and the emitter
 * func_801F2FC4 reads the colour, angle and position fields, so only those
 * offsets plus the state bytes this sweep advances are modelled here.
 */
typedef struct Area12513WorkRecord {
  u8 flags_00;
  u8 state_01;
  u8 counter_02;
  u8 intensity_03;
  u16 angle_04;
  u8 pad_06[2];
  s32 position_08;
  s32 position_0c;
  s32 scaled_10;
  u8 pad_14[4];
  s32 velocity_18;
  s32 velocity_1c;
  u8 pad_20[8];
} Area12513WorkRecord;

/* @source 0x801F33B0
 * @behavior Per-frame sweep of the 0x10 records of the 0x28-byte-stride work
 *           table at 0x800E4800: for each record whose lead byte at offset 0 is
 *           non-zero it advances the two-state intensity ramp, raising the byte
 *           at +0x03 by 4 with the counter byte at +0x02 while the state byte at
 *           +0x01 is 0 and lowering it by 4 once that counter reaches 0x10,
 *           until the counter reaches 0x20 and the lead byte is cleared; then it
 *           adds the velocity word at +0x18 to the position word at +0x08,
 *           advances the angle halfword at +0x04 by 8, adds the velocity word at
 *           +0x1C to the position word at +0x0C, stores func_8015477C of the two
 *           position words shifted left 16 into the word at +0x10, and calls
 *           this overlay's record emitter func_801F2FC4 with the record. It
 *           returns 1 once at least one record was active and 0 otherwise; the
 *           0x58-byte frame holds the scratch area func_801AFE18 fills at
 *           $sp+0x10 plus the saved $ra/$s0-$s3 of the sweep.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 rampIntensityIntegrateAndRenderActiveWorkRecordsWorld03Area12513_801F33B0(
    void) {
  Area12513WorkRecord* record;
  u8 scratch[0x30];
  u8 i;
  u8 handled;
  u8 count;
  s32 y;

  func_801AFE18(scratch);

  handled = 0;
  record = PSX_PTR(Area12513WorkRecord, 0x800E4800u);
  i = 0;
  do {
    if (record->flags_00 != 0) {
      handled = 1;
      switch (record->state_01) {
      case 0:
        record->intensity_03 += 4;
        count = record->counter_02 += 1;
        if (count == 0x10) {
          record->state_01 += 1;
        }
        break;
      case 1:
        record->intensity_03 -= 4;
        count = record->counter_02 += 1;
        if (count == 0x20) {
          record->flags_00 = 0;
        }
        break;
      }
      record->position_08 += record->velocity_18;
      record->angle_04 += 8;
      y = record->position_0c + record->velocity_1c;
      record->position_0c = y;
      record->scaled_10 = func_8015477C(record->position_08, y) << 16;
      func_801F2FC4(record);
    }
    record++;
    i++;
  } while (i < 0x10);

  return handled;
}
