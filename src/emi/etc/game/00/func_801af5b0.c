#include "internal.h"

#define GAME_RAM_BASE      ((volatile u8*)0x80140000u)
#define GAME_CHARACTER_MAP ((const volatile u8*)0x80144f5au)
#define GAME_ABILITY_MAP   ((const volatile u8*)0x80181b10u)

extern s32 func_80166A10(u8 character_index, u8 ability_kind, u8 mode);

/* @behavior checks whether an ability kind is currently selectable for a
 * character/category, applying learned-level, flag, and story-state gates.
 * @source 0x801AF5B0
 * @see docs/specs/data/equipment.md
 */
s32 func_801AF5B0(s32 item_category, s32 character_index, s32 ability_kind) {
  s32                           saved_kind;
  s32                           saved_character;
  const volatile AbilityObject* ability;
  u8                            mapped_character;
  u32                           character_offset;
  u32                           learned_level;
  u32                           required_level;

  saved_kind = ability_kind;
  saved_character = character_index;
  if ((u8)saved_kind == 0u) {
    return 0;
  }

  ability = &ABILITY_OBJECTS[(u8)saved_kind];

  if ((u8)item_category == 1u) {
    mapped_character =
        GAME_ABILITY_MAP[GAME_CHARACTER_MAP[(u8)saved_character]];
    character_offset = (u32)mapped_character * 0xa4u;
    required_level =
        func_80166A10(mapped_character, (u8)saved_kind, 0u) & 0xffu;
    learned_level =
        *(const volatile u16*)(GAME_RAM_BASE + character_offset + 0x497eu);
    if (learned_level < required_level) {
      return 0;
    }
    if (ability->targeting_flags & 1u) {
      return 1;
    }
    return 0;
  } else if ((u8)item_category < 2u) {
    return 1;
  } else if ((u8)item_category != 2u) {
    return 1;
  }

  character_offset = (u32)(u8)saved_character * 0x140u;
  required_level =
      func_80166A10((u8)saved_character, (u8)saved_kind, 1u) & 0xffu;
  learned_level =
      *(const volatile u16*)(GAME_RAM_BASE + character_offset + 0x5f1au);
  if (learned_level < required_level) {
    return 0;
  }

  if ((ability->targeting_flags & 2u) == 0u) {
    return 0;
  }
  if ((*(const volatile u16*)(GAME_RAM_BASE + character_offset + 0x5f10u) &
       0x10u) != 0u &&
      (*(const volatile u16*)&ability->element & 0x400u) != 0u) {
    return 0;
  }

  if ((u8)saved_kind == 20u &&
      *(const volatile u8*)(GAME_RAM_BASE + (u32)(u8)saved_character * 0x140u +
                            0x5f1eu) >= 5u) {
    return 0;
  }
  if ((u8)saved_kind == 21u && *(const volatile u32*)0x80145558u != 0u) {
    return 0;
  }
  if ((u8)saved_kind == 62u && *(const volatile u32*)0x80145554u != 0u) {
    return 0;
  }
  if ((u8)saved_kind == 140u &&
      *(const volatile u16*)(GAME_RAM_BASE + (u32)(u8)saved_character * 0x140u +
                             0x5f1au) != 0u) {
    return 0;
  }
  if ((u8)saved_kind == 151u && *(const volatile u8*)0x801462eau == 0x25u) {
    return 0;
  }

  return 1;
}
