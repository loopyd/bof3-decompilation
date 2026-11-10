#include "bof3/bof3.h"

extern int rand(void);

/* Scratchpad pointer cell 0x1F800044: it publishes the work record this overlay
 * reads its emitter origin from, and this target's own symbol map binds it. The
 * cell sits outside every overlay payload, so the loads below stay shared with
 * the rest of the corpus. */
extern u8 *D_1F800044;

/* One 0x1C-byte entry of the 0x100-entry effect table at 0x800E4800. The lead
 * byte marks an entry live, the three bytes behind it hold the frame countdown,
 * the animation state the switch dispatches on and the direction flag that
 * picks the sign of the offsets the states apply, and the words hold the motion
 * the state steps integrate. */
typedef struct Area12013EffectRecord {
  u8 flags_00;
  u8 timer_01;
  u8 state_02;
  u8 dir_03;
  s32 speed_04;
  u16 angle_08;
  u8 pad_0a[2];
  s32 pos_x_0c;
  s32 pos_y_10;
  s32 vel_14;
  u8 pad_18[4];
} Area12013EffectRecord;

/* 0x801AFE18 - shared projection setup in the main executable: it fills the
 * 0x20-byte stack scratch area this frame passes at $sp+0x10 and only a0 is
 * live at the call site. */
void func_801AFE18(void *arg0);

/* 0x801F3468 - this overlay's per-record emitter. It builds its draw mode from
 * the record word pair at +0x0C and +0x10, so it takes the entry pointer. */
void func_801F3468(Area12013EffectRecord *record);

/* 0x801783C8 and 0x801782FC - the main executable's angle pair. Both take the
 * record angle halfword and return the 0x10000-scaled offset the states scale
 * and add to the emitter origin. */
s32 func_801783C8(u16 angle);
s32 func_801782FC(u16 angle);

/* @source 0x801F36B0
 * @behavior Per-frame sweep of the 0x100 entries of the 0x1C-byte-stride effect
 *           table at 0x800E4800, the work area this overlay shares with the
 *           world and scenario overlays. For every entry whose lead byte at
 *           offset 0 is non-zero it runs the state the byte at +0x02 selects
 *           through the five-entry jump table at 0x801F2C04: state 0 seeds the
 *           entry from the work record published through pointer cell
 *           0x1F800044, copying that record word +0x34 into the entry word
 *           +0x0C, adding the record word +0x38 with a 0x28000 spread whose
 *           sign the direction byte at +0x03 picks into +0x10, adding 0x1800000
 *           to the record word +0x3C into +0x14, storing (rand & 0x1FF) - 0x180
 *           shifted left sixteen and divided by 32 into +0x04, setting the
 *           countdown at +0x01 to 0x20 and advancing the state; state 1
 *           subtracts 0x2C00 from +0x0C, adds plus or minus 0xC00 to +0x10 by
 *           the direction byte, adds the +0x04 speed word to the +0x14 word and,
 *           once the countdown reaches zero, rewrites the angle at +0x08 with
 *           (rand & 0x1F) and 0x400 or 0xFC00 by direction, reloads the
 *           countdown with 0x20 and advances the state by work-record byte 0x06
 *           plus one; states 2 and 3 place +0x0C and +0x10 from the work-record
 *           origin added to sixteen times the two angle lookups at +0x08 and
 *           lower +0x0C by 0x58000, rotate +0x08 by plus or minus 0x80 by
 *           direction inside 0xFFF, and then diverge: state 2 retires the entry
 *           by clearing the lead byte once the countdown expires, while state 3
 *           reseeds +0x04 with (rand & 0x7F) + 0x80, the countdown with 0x10 and
 *           advances the state when work byte 0x06 reads 2; state 4 integrates
 *           +0x0C and +0x10 by the +0x04 speed times the angle lookups shifted
 *           right four and retires the entry once the countdown expires. Every
 *           live entry is then passed to the emitter at 0x801F3468 and marks the
 *           frame handled, and the sweep returns 1 when at least one entry was
 *           live and 0 when none was. It takes no arguments and its 0x40-byte
 *           frame holds the 0x20-byte scratch area func_801AFE18 fills at
 *           $sp+0x10 plus the saved $s0-$s2 and $ra.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 advanceWorkRecordStatesAndRenderActiveWorld03Area12013_801F36B0(void) {
  Area12013EffectRecord *record;
  u8 scratch[0x20];
  u16 i;
  s32 handled;

  func_801AFE18(scratch);

  handled = 0;
  record = PSX_PTR(Area12013EffectRecord, 0x800E4800u);
  i = 0;
  do {
    if (record->flags_00 != 0) {
      switch (record->state_02) {
      case 0:
        record->timer_01 = 0x20;
        record->pos_x_0c = FIELD_REF(s32, D_1F800044, 0x34u);
        record->pos_y_10 = FIELD_REF(s32, D_1F800044, 0x38u) +
                           (record->dir_03 != 0 ? 0x28000 : -0x28000);
        record->vel_14 = FIELD_REF(s32, D_1F800044, 0x3Cu) + 0x1800000;
        record->speed_04 = (((rand() & 0x1FF) - 0x180) << 16) / 32;
        record->state_02 += 1;
        break;
      case 1:
        record->pos_x_0c -= 0x2C00;
        record->pos_y_10 += (record->dir_03 != 0 ? -0xC00 : 0xC00);
        record->vel_14 += record->speed_04;
        if (--record->timer_01 == 0) {
          record->angle_08 = (rand() & 0x1F) | (record->dir_03 != 0 ? 0x400 : 0xFC00);
          record->state_02 += FIELD_REF(u8, D_1F800044, 0x06u) + 1;
          record->timer_01 = 0x20;
        }
        break;
      case 2:
        record->pos_x_0c = (func_801783C8(record->angle_08) << 4) +
                           FIELD_REF(s32, D_1F800044, 0x34u) - 0x58000;
        record->pos_y_10 =
            (func_801782FC(record->angle_08) << 4) + FIELD_REF(s32, D_1F800044, 0x38u);
        record->angle_08 = (record->angle_08 + (record->dir_03 != 0 ? 0x80 : -0x80)) &
                           0xFFF;
        if (--record->timer_01 == 0) {
          record->flags_00 = 0;
        }
        break;
      case 3:
        record->pos_x_0c = (func_801783C8(record->angle_08) << 4) +
                           FIELD_REF(s32, D_1F800044, 0x34u) - 0x58000;
        record->pos_y_10 =
            (func_801782FC(record->angle_08) << 4) + FIELD_REF(s32, D_1F800044, 0x38u);
        record->angle_08 = (record->angle_08 + (record->dir_03 != 0 ? 0x80 : -0x80)) &
                           0xFFF;
        if (FIELD_REF(u8, D_1F800044, 0x06u) == 2) {
          record->speed_04 = (rand() & 0x7F) + 0x80;
          record->timer_01 = 0x10;
          record->state_02 += 1;
        }
        break;
      case 4:
        record->pos_x_0c += (record->speed_04 * func_801783C8(record->angle_08)) >> 4;
        record->pos_y_10 += (record->speed_04 * func_801782FC(record->angle_08)) >> 4;
        if (--record->timer_01 == 0) {
          record->flags_00 = 0;
        }
        break;
      }
      func_801F3468(record);
      handled = 1;
    }
    record++;
    i++;
  } while (i < 0x100);

  return handled;
}
