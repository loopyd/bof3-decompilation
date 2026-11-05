#include "bof3/bof3.h"
#include <rand.h>

/* @source 0x1F800044 @kind unknown */
extern u8 *scenarioScena0900Work;

/* 0x801AFE18 — shared projection setup consuming a 0x20-byte stack scratch
 * area; only a0 is live at the call site. */
void func_801AFE18(void *arg0);

/* 0x801F7424 — this overlay's per-record emitter: it draws one translucent
 * grid-cell quad from the record's position words at 0x18 and 0x1C and its
 * colour/pattern bytes at 0x03 and 0x04. */
void func_801F7424(void *record);

/* One 0x28-byte record of the 32-record effect bank at 0x800E5000 whose bank
 * is selected by scratchpad work byte 6 with stride 0x500. The seeding helper
 * func_801F76D0 fills the position and velocity words and the emitter
 * func_801F7424 reads the colour, pattern and position fields, so only the
 * offsets those three functions touch are modelled here.
 */
typedef struct ScenarioScena0900EffectRecord {
  u8 flags_00;
  u8 pad_01;
  u8 life_02;
  u8 fade_03;
  u8 frame_04;
  u8 pad_05[3];
  s32 velocity_08;
  s32 velocity_0c;
  s32 velocity_10;
  u8 pad_14[4];
  s32 position_18;
  s32 position_1c;
  s32 position_20;
  u8 pad_24[4];
} ScenarioScena0900EffectRecord;

/* @source 0x801F75BC
 * @behavior Integrates and emits the active records of the 0x800E5000 effect
 * bank selected by scratchpad work byte 6 (bank stride 0x500): it sweeps the
 * 0x20 records of 0x28-byte stride, and for each record whose lead byte at
 * offset 0 is non-zero it adds the velocity words at 0x08, 0x0C and 0x10 to
 * the position words at 0x18, 0x1C and 0x20, applies the -0x40000 gravity step
 * to the 0x10 velocity, decrements the fade byte at 0x03 by 4, reseeds the
 * pattern byte at 0x04 from `rand() & 7`, decrements the countdown byte at
 * 0x02 and clears the lead byte when that countdown reaches zero, then calls
 * the record emitter func_801F7424 with the record. It takes no arguments and
 * returns 1 once at least one active record was handled and 0 otherwise; the
 * 0x40-byte frame holds the 0x20-byte scratch area func_801AFE18 fills plus
 * the saved $ra/$s0-$s2 of the sweep.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 advanceAndRenderActiveRecordsInSelectedBankScenarioScena0900_801F75BC(void) {
  ScenarioScena0900EffectRecord *record;
  u8 scratch[0x20];
  u8 i;
  s32 handled;

  func_801AFE18(scratch);

  handled = 0;
  record = (ScenarioScena0900EffectRecord *)(
      PSX_PTR(u8, 0x800E5000u) + scenarioScena0900Work[6] * 0x500);
  i = 0;
  do {
    if (record->flags_00 != 0) {
      record->position_18 += record->velocity_08;
      record->position_1c += record->velocity_0c;
      record->position_20 += record->velocity_10;
      record->velocity_10 -= 0x40000;
      record->fade_03 -= 4;
      record->frame_04 = (u8)(rand() & 7);
      if (--record->life_02 == 0) {
        record->flags_00 = 0;
      }
      func_801F7424(record);
      handled = 1;
    }
    record++;
    i++;
  } while (i < 0x20);

  return handled;
}
