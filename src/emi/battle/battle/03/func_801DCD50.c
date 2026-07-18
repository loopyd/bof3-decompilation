#include "internal.h"

/* @behavior applies the current scratchpad damage modifiers, variance table, and
 * optional battler-specific scale table to one signed damage value.
 * @source 0x801DCD50
 */
u32 func_801DCD50(u32 arg0, u8 arg1, s32 arg2) {
  s32 value;
  s32 scale;
  u16 scratch_flags;

  value = ((((arg2 * 0x100) / 0x3e800) * -0x20000) + 0x100) >> 8;
  if (value < 0xcdu) {
    value = 0xcdu;
  }

  value = (((arg2 * 0x100) * value) >> 8) *
              BATTLE_VARIANCE_TABLE_AFA0[func_8017E3D4() & 7u] >>
          8;

  scratch_flags = *(volatile u16*)0x1f800000u;
  if ((scratch_flags & 0x1fu) != 0u) {
    value = (value * (s16)func_800A2AE0(arg1)) / 100;
  }
  if ((scratch_flags & 0x20u) != 0u) {
    if (arg1 < 3u) {
      value = (value *
               BATTLE_SCALE_TABLE_AFC0[BATTLE_LOCAL_BYTE_120(
                                           &BATTLE_LOCAL_WORK_ARRAY[arg1]) >>
                                       7]) /
              100;
    } else {
      value = (value * BATTLE_SCALE_TABLE_AFC0[BATTLE_ENEMY_BYTE_114(
                           &BATTLE_ENEMY_WORK_ARRAY[(arg1 - 3u) & 0xffu])]) /
              100;
    }
  }

  if ((value & 0xff) >= 0x80) {
    value += 0x100;
  }
  return (u32)(value >> 8);
}
