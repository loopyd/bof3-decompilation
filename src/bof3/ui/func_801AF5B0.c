#include "bof3/ui/game00_internal.h"

extern s32 func_80166A10(u8 character_index, u8 ability_kind, u8 mode);

/* @behavior checks whether an ability kind is currently selectable for a
 * character/category, applying learned-level, flag, and story-state gates.
 * Being selectable for the party category (1) resolves the mapped character
 * through D_80181B10[D_80144F5A[character]] and the 0xA4-byte-stride character
 * record, then requires the learned level to reach the level func_80166A10
 * reports and the ability's targeting flags to carry bit 0. The battle
 * category (2) instead reads the 0x140-byte-stride character record, requires
 * targeting bit 1, rejects an element-locked character whose ability carries
 * 0x400, and finally applies the per-kind story gates: kind 20 needs the level
 * byte below 5, kind 21 and kind 62 need their front-end countdown word clear,
 * kind 140 needs the learned level clear, and kind 151 needs the scenario byte
 * D_801462EA to stay away from 0x25.
 * @source 0x801AF5B0
 * @see docs/specs/data/equipment.md
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801AF5B0(s32 item_category, s32 character_index, s32 ability_kind) {
  const volatile AbilityObject* ability;
  u8                            mapped_character;
  u32                           character_offset;
  u32                           battle_character;
  u32                           battle_character_offset;
  u32                           learned_level;
  u32                           required_level;
  u8                            character;
  u8                            kind;

  character = (u8)character_index;
  kind = (u8)ability_kind;
  if (kind == 0u) {
    return 0;
  }

  ability = &abilityObjects[kind];

  do {
    item_category = (u8)item_category;
  } while (0);
  if (item_category == 1) {
    goto party_category;
  }
  if (item_category < 2) {
    goto selectable;
  }
  if (item_category == 2) {
    goto battle_category;
  }
  goto selectable;

party_category:
  mapped_character =
      D_80181B10[D_80144F5A[character]];
  character_offset = (u32)mapped_character << 2;
  character_offset += mapped_character;
  character_offset <<= 3;
  character_offset += mapped_character;
  character_offset <<= 2;
  required_level =
      func_80166A10(mapped_character, kind, 0u) & 0xffu;
  learned_level = *(const u16*)&D_8014497E[character_offset];
  learned_level = learned_level < required_level;
  if (learned_level != 0u) {
    return 0;
  }
  if ((ability->targeting_flags & 1u) != 0u) {
    goto selectable;
  }
  goto unselectable;

battle_category:
  battle_character = (u32)character;
  battle_character_offset = battle_character * 0x140u;
  required_level =
      func_80166A10(battle_character, kind, 1u) & 0xffu;
  learned_level = *(const u16*)&D_80145F1A[battle_character_offset];
  learned_level = learned_level < required_level;
  if (learned_level != 0u) {
    return 0;
  }

  if ((ability->targeting_flags & 2u) == 0u) {
    return 0;
  }
  if ((*(const u16*)&D_80145F10[battle_character_offset] & 0x10u) != 0u &&
      (*(const u16*)&ability->element & 0x400u) != 0u) {
    return 0;
  }

  if (kind == 20u) {
    u32 story_character_level;

    do {
      story_character_level =
          D_80145F1E[(u32)character * 0x140u];
    } while (0);
    if (story_character_level >= 5u) {
      return 0;
    }
  }
  if (kind == 21u && GAME_UNK_80145558 != 0u) {
    return 0;
  }
  if (kind == 62u && GAME_UNK_80145554 != 0u) {
    return 0;
  }
  if (kind == 140u) {
    u32 story_learned_level;

    do {
      story_learned_level =
          *(const u16*)&D_80145F1A[(u32)character * 0x140u];
    } while (0);
    if (story_learned_level != 0u) {
      return 0;
    }
  }
  if (kind != 151u) {
    goto selectable;
  }
  if (GAME_UNK_801462EA != 0x25u) {
    goto selectable;
  }
unselectable:
  return 0;

selectable:
  return 1;
}
