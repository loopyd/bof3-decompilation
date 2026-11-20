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
 * @status partial
 * @match 85.81
 * @residual 127 of 148 instructions at the original 592-byte size. Saved
 * register allocation places the character index in s3, ability kind in s1
 * and ability pointer in s2, against s1, s2 and s3 in the original. Prologue
 * order and argument setup around the battle-level query also differ.
 * Complete byte comparison still differs.
 */
s32 func_801AF5B0(s32 item_category, s32 character_index, s32 ability_kind) {
  const volatile AbilityObject* ability;
  u8                            mapped_character;
  u32                           character_offset;
  u32                           learned_level;
  u32                           required_level;
  u8                            character;

  character = (u8)character_index;
  if ((u8)ability_kind == 0u) {
    return 0;
  }

  ability = &abilityObjects[(u8)ability_kind];

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
      func_80166A10(mapped_character, (u8)ability_kind, 0u) & 0xffu;
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
  character_offset = (u32)character * 0x140u;
  required_level =
      func_80166A10(character, (u8)ability_kind, 1u) & 0xffu;
  learned_level = *(const u16*)&D_80145F1A[character_offset];
  learned_level = learned_level < required_level;
  if (learned_level != 0u) {
    return 0;
  }

  if ((ability->targeting_flags & 2u) == 0u) {
    return 0;
  }
  if ((*(const u16*)&D_80145F10[character_offset] & 0x10u) != 0u &&
      (*(const u16*)&ability->element & 0x400u) != 0u) {
    return 0;
  }

  if ((u8)ability_kind == 20u &&
      D_80145F1E[(u32)character * 0x140u] >= 5u) {
    return 0;
  }
  if ((u8)ability_kind == 21u && GAME_UNK_80145558 != 0u) {
    return 0;
  }
  if ((u8)ability_kind == 62u && GAME_UNK_80145554 != 0u) {
    return 0;
  }
  if ((u8)ability_kind == 140u &&
      *(const u16*)&D_80145F1A[(u32)character * 0x140u] != 0u) {
    return 0;
  }
  if ((u8)ability_kind != 151u) {
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
