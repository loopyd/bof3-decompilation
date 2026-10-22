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
 * @match 58.78
 * @residual 87 of 148 instructions at 572 bytes against the original 592. The
 * dispatch chain, both level queries, the ability-record fields and the three
 * fixed-address story gates match: D_80145558, D_80145554 and D_801462EA are
 * read through non-volatile constant-address macros, which folds each access to
 * the original's `lui` + offset form. Remaining classes are allocation and
 * return-tail if conversion: the prologue saves the ability kind into s1 where
 * the original saves the character index into s1 and the ability kind into s2
 * (first mismatch at instruction 2; the ability pointer then takes s2 here
 * against s3 there), the kind-140 gate compiles as an inverted `beqz` plus `j`
 * where the original branches `bnez` to the shared return, and the kind-151
 * gate folds to `xori`/`sltu` where the original keeps a `bne` pair. Measured
 * and rejected on this allocation: repeated `(u8)` masks of the parameter
 * instead of the character local, a 32-bit character local, the ability
 * pointer built before the zero-kind test, nested `if` forms of the kind-140
 * and kind-151 gates, a switch on the masked category with and without the
 * guard-return tail, the unmasked category compare (drops the original's
 * `andi`), and the earlier if/else, offset-reuse and single-expression level
 * variants, all of which measure below this candidate's 87 matched
 * instructions; the wave-13 lane report carries the full ledger.
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

  if ((u8)item_category == 1u) {
    mapped_character =
        D_80181B10[D_80144F5A[character]];
    character_offset = (u32)mapped_character * 0xa4u;
    required_level =
        func_80166A10(mapped_character, (u8)ability_kind, 0u) & 0xffu;
    learned_level = *(const u16*)&D_8014497E[character_offset];
    if (learned_level < required_level) {
      return 0;
    }
    if ((ability->targeting_flags & 1u) != 0u) {
      return 1;
    }
    return 0;
  } else if ((u8)item_category < 2) {
    return 1;
  } else if ((u8)item_category != 2) {
    return 1;
  }

  character_offset = (u32)character * 0x140u;
  required_level =
      func_80166A10(character, (u8)ability_kind, 1u) & 0xffu;
  learned_level = *(const u16*)&D_80145F1A[character_offset];
  if (learned_level < required_level) {
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
    return 1;
  }
  if (GAME_UNK_801462EA == 0x25u) {
    return 0;
  }

  return 1;
}
