#ifndef BATTLE_ABILITY_H
#define BATTLE_ABILITY_H

#include "base/types.h"

/* 0x14-byte record, 228 entries at 0x801CA70C.
 * Offsets 0x10-0x11 are element/ability_flags (menu/effect code) or a
 * 16-bit selection_mask (battle selector). See docs/specs/data/encoding.md. */
typedef struct AbilityObject {
  u8  name[0x0c];
  u8  targeting_flags;
  u8  skill_type;
  u8  cost;
  u8  power;
  u8  element;
  u8  ability_flags;
  u8  control_12[2];
} AbilityObject;

#endif
