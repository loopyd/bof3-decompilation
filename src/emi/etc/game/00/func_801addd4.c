#include "internal.h"

#define GAME_RAM_BASE   ((volatile u8*)0x80140000u)
#define GAME_LEVEL_BASE ((const volatile u8*)0x801d0000u)

/* @behavior applies each newly reached level's packed growth row to the mutable
 * character record, clamps the six base stats to 0..999, registers both ability
 * bytes, then recalculates the character.
 * @source 0x801addd4 FUN_801addd4
 * @see docs/specs/data/characters.md
 */
void func_801addd4(s32 arg0) {
  s32 character_value;
  u8  character;
  u8  current_level;
  u8  new_level;
  u8  level;
  u32 character_offset;
  u32 level_offset;
  s32 exp;
  s32 exp_total;
  s32 delta;
  s32 value;

  character_value = arg0;
  character = (u8)character_value;
  character_offset = (u32)character * 0xa4u;

  new_level = 1u;
  exp_total = 0;
  level_offset = (u32)character * 0x318u;
  exp = *(volatile u32*)(GAME_RAM_BASE + character_offset + 0x4970u);
  while (new_level < 99u) {
    exp_total += *(const volatile u16*)(GAME_LEVEL_BASE + level_offset +
                                        (u32)new_level * 8u - 0x4724u);
    if (exp < exp_total) {
      break;
    }
    new_level++;
  }

  current_level = *(volatile u8*)(GAME_RAM_BASE + character_offset + 0x496eu);
  if (current_level == 99u || new_level <= current_level) {
    return;
  }

  level = current_level;
  while (level < new_level) {
    delta =
        (s32)(s8) * (volatile u8*)(GAME_RAM_BASE + character_offset + 0x49edu) +
        (s32) * (const volatile u8*)(GAME_LEVEL_BASE + level_offset +
                                     (u32)level * 8u - 0x4722u);
    if (delta < 0) {
      delta = 0;
    }
    value =
        (s32) * (volatile u16*)(GAME_RAM_BASE + character_offset + 0x49a4u) +
        delta;
    *(volatile u16*)(GAME_RAM_BASE + character_offset + 0x49a4u) = (u16)value;
    if ((u16)value >= 1000u) {
      *(volatile u16*)(GAME_RAM_BASE + character_offset + 0x49a4u) = 999u;
    }

    delta =
        (s32)(s8) * (volatile u8*)(GAME_RAM_BASE + character_offset + 0x49eeu) +
        (s32) * (const volatile u8*)(GAME_LEVEL_BASE + level_offset +
                                     (u32)level * 8u - 0x4721u);
    if (delta < 0) {
      delta = 0;
    }
    value =
        (s32) * (volatile u16*)(GAME_RAM_BASE + character_offset + 0x49a6u) +
        delta;
    *(volatile u16*)(GAME_RAM_BASE + character_offset + 0x49a6u) = (u16)value;
    if ((u16)value >= 1000u) {
      *(volatile u16*)(GAME_RAM_BASE + character_offset + 0x49a6u) = 999u;
    }

    delta =
        (s32)(s8) * (volatile u8*)(GAME_RAM_BASE + character_offset + 0x49efu) +
        (s32)(*(const volatile u8*)(GAME_LEVEL_BASE + level_offset +
                                    (u32)level * 8u - 0x4720u) >>
              4);
    if (delta < 0) {
      delta = 0;
    }
    value =
        (s32) * (volatile u16*)(GAME_RAM_BASE + character_offset + 0x49a8u) +
        delta;
    *(volatile u16*)(GAME_RAM_BASE + character_offset + 0x49a8u) = (u16)value;
    if ((u16)value >= 1000u) {
      *(volatile u16*)(GAME_RAM_BASE + character_offset + 0x49a8u) = 999u;
    }

    delta =
        (s32)(s8) * (volatile u8*)(GAME_RAM_BASE + character_offset + 0x49f0u) +
        (s32)(*(const volatile u8*)(GAME_LEVEL_BASE + level_offset +
                                    (u32)level * 8u - 0x4720u) &
              0x0fu);
    if (delta < 0) {
      delta = 0;
    }
    value =
        (s32) * (volatile u16*)(GAME_RAM_BASE + character_offset + 0x49aau) +
        delta;
    *(volatile u16*)(GAME_RAM_BASE + character_offset + 0x49aau) = (u16)value;
    if ((u16)value >= 1000u) {
      *(volatile u16*)(GAME_RAM_BASE + character_offset + 0x49aau) = 999u;
    }

    delta =
        (s32)(s8) * (volatile u8*)(GAME_RAM_BASE + character_offset + 0x49f1u) +
        (s32)(*(const volatile u8*)(GAME_LEVEL_BASE + level_offset +
                                    (u32)level * 8u - 0x471fu) >>
              4);
    if (delta < 0) {
      delta = 0;
    }
    value =
        (s32) * (volatile u16*)(GAME_RAM_BASE + character_offset + 0x49acu) +
        delta;
    *(volatile u16*)(GAME_RAM_BASE + character_offset + 0x49acu) = (u16)value;
    if ((u16)value >= 1000u) {
      *(volatile u16*)(GAME_RAM_BASE + character_offset + 0x49acu) = 999u;
    }

    delta =
        (s32)(s8) * (volatile u8*)(GAME_RAM_BASE + character_offset + 0x49f2u) +
        (s32)(*(const volatile u8*)(GAME_LEVEL_BASE + level_offset +
                                    (u32)level * 8u - 0x471fu) &
              0x0fu);
    if (delta < 0) {
      delta = 0;
    }
    value =
        (s32) * (volatile u16*)(GAME_RAM_BASE + character_offset + 0x49aeu) +
        delta;
    *(volatile u16*)(GAME_RAM_BASE + character_offset + 0x49aeu) = (u16)value;
    if ((u16)value >= 1000u) {
      *(volatile u16*)(GAME_RAM_BASE + character_offset + 0x49aeu) = 999u;
    }

    func_801651dc(*(const volatile u8*)(GAME_LEVEL_BASE + level_offset +
                                        (u32)level * 8u - 0x471eu),
                  character, 0, 0);
    func_801651dc(*(const volatile u8*)(GAME_LEVEL_BASE + level_offset +
                                        (u32)level * 8u - 0x471du),
                  character, 0, 0);
    level++;
  }

  *(volatile u8*)(GAME_RAM_BASE + character_offset + 0x496eu) = new_level;
  func_80164a44((volatile u8*)(GAME_RAM_BASE + character_offset + 0x4968u));
}
