#include "bof3/battle/battle03_internal.h"

extern int rand(void);

/* @source 0x801E2024
 * @behavior Middle handler of the three-entry default-class dispatch table at
 * 0x801EB27C: counts down the scratch work byte at +0x09 and returns while it is
 * nonzero. Once that countdown expires it asks the gate func_800A8F58 for its low
 * byte and, while the gate stays clear, reads the unsigned chance byte at +0xAA of
 * the D_80146250 record and rolls `rand() % 100` against it; a roll below the
 * chance raises bit 0x80 of the battle global halfword 0x801462E8 (the flag
 * func_801DC73C reads to return full damage instead of running its own percent
 * roll) and submits effect variant 3, while a roll at or above the chance submits
 * variant 2 instead. A set gate low byte skips the roll and submits variant 3.
 * Both cases then submit effect variant 4, reload the scratch work pointer and
 * increment scratch work byte +0x02, which advances the default-class dispatch to
 * raiseFlag4AndUpdate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void submitRolledEffectAndAdvance(void) {
  u8   chance;
  u8*  record;
  u16* flags;
  u32  kind;

  if (--D_1F800044->unk_09 != 0u) {
    return;
  }
  if (func_800A8F58() != 0u) {
    kind = 3u;
  } else {
    record = (u8*)D_80146250;
    chance = record[0xAA];
    if (rand() % 100 < chance) {
      flags = (u16*)&BATTLE_GLOBAL_HALF_62E8;
      *flags |= 0x80u;
      kind = 3u;
    } else {
      kind = 2u;
    }
  }
  submitCurrentWorkEffectVariant(kind);
  submitCurrentWorkEffectVariant(4u);
  D_1F800044->unk_02++;
}
