#include "internal.h"

/* @behavior applies damage modifiers for a battler's attack, incorporating element
 * affinity, randomness, defended status, and battle formation effects.
 * returns the final damage value as s16.
 * @source 0x800A2880
 */
s16 func_800A2880(u8 battler_index, u16 base_value, u8 element_flag) {
  volatile s16* modifier_table;
  volatile u8*  player_state;
  u16           rule_selection;
  u32           adjusted_value;
  u32           random_bonus;
  s16           element_mod;
  u16           raw_def_rate;
  u32           scale;
  u32           defence_bonus;
  s16           result;

  raw_def_rate = PSX_REF(volatile u16, 0x801ec312u) + 0x64u;
  scale = (raw_def_rate * 0x64u) / 100u;

  rule_selection = PSX_REF(volatile u16, 0x801463c0u);
  rule_selection = (rule_selection * 0x14u) + 0x801d0000u;
  rule_selection = PSX_REF(volatile u16, rule_selection - 0x58e4u) & 0x1ffu;

  adjusted_value = ((u32)base_value * scale) / 100u;
  scale = PSX_REF(volatile u16, 0x801ec2f2u) / 5u;
  if ((s32)(0x64u - scale) < 0x32) {
    scale = 0x32u;
  } else {
    scale = 0x64u - scale;
  }
  adjusted_value = (adjusted_value * scale) / 100u;

  if (rule_selection != 0u) {
    if (element_flag != 0u) {
      element_mod = FUNCTION_AT(s16 (*)(u8, u16, u32), 0x800a2ef0u)(
          battler_index, rule_selection, 0x51eb851fu);
    } else {
      element_mod = func_800A2AE0(battler_index, rule_selection);
    }
    adjusted_value = (adjusted_value * (u16)element_mod) / 100u;
  }

  random_bonus = FUNCTION_AT(u32 (*)(void), 0x8017e3d4u)();
  modifier_table = (volatile s16*)((u32)BATTLE_SELECTION_TABLE_BASE +
                                   (((random_bonus & 7u) * 2u) + 0x492cu));
  defence_bonus = (adjusted_value * (u32)*modifier_table) / 10000u;

  if ((PSX_REF(volatile u8, 0x80146394u) < 3u) && (PSX_REF(volatile u8, 0x80144f58u) == 2u) &&
      (defence_bonus > 0u)) {
    defence_bonus -= (adjusted_value * (u32)*modifier_table) / 40000u;
  }

  player_state = BATTLE_GAME_RAM_BASE;
  if (battler_index < 3u) {
    defence_bonus /= 2u;
    if (PSX_REF(volatile u32, 0x80145fb4u + ((u32)battler_index * 0x140u) + 4u) & 0x200u) {
      defence_bonus /= 2u;
    }
    if (PSX_REF(volatile u32, 0x80145fb4u + ((u32)battler_index * 0x140u)) & 0x10000u) {
      result = 0;
    } else {
      result = (s16)defence_bonus;
    }
  } else {
    defence_bonus /= 2u;
    if (PSX_REF(volatile u32, 0x801eb6d0u + ((u32)(battler_index - 3u) * 0x118u) + 4u) &
        0x200u) {
      defence_bonus /= 2u;
    }
    if (PSX_REF(volatile u32, 0x801eb6d0u + ((u32)(battler_index - 3u) * 0x118u)) & 0x10000u) {
      result = 0;
    } else {
      result = (s16)defence_bonus;
    }
  }

  return result;
}
