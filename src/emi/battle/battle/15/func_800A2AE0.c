#include "internal.h"

/* @behavior sums elemental resistance modifiers for a battler based on a bitmask
 * of active elements. each element slot in the battler's data holds an index
 * into a s16 modifier table at 0x800b493c.
 * @source 0x800A2AE0
 */
s16 func_800A2AE0(u8 battler_index, u16 element_mask) {
  volatile s16* modifier_table;
  s16           result;
  u8            element_slot;

  modifier_table = (volatile s16*)0x800b493cu;
  result = 0;

  if (battler_index < 3u) {
    if (element_mask & 1u) {
      element_slot = MMIO8(0x80145f2fu + ((u32)battler_index * 0x140u));
      result += modifier_table[element_slot];
    }
    if (element_mask & 2u) {
      element_slot = MMIO8(0x80145f30u + ((u32)battler_index * 0x140u));
      result += modifier_table[element_slot];
    }
    if (element_mask & 4u) {
      element_slot = MMIO8(0x80145f31u + ((u32)battler_index * 0x140u));
      result += modifier_table[element_slot];
    }
    if (element_mask & 8u) {
      element_slot = MMIO8(0x80145f32u + ((u32)battler_index * 0x140u));
      result += modifier_table[element_slot];
    }
    if (element_mask & 0x10u) {
      element_slot = MMIO8(0x80145f33u + ((u32)battler_index * 0x140u));
      result += modifier_table[element_slot];
    }
  } else {
    if (element_mask & 1u) {
      element_slot = MMIO8(0x801eb6dfu + ((u32)(battler_index - 3u) * 0x118u));
      result += modifier_table[element_slot];
    }
    if (element_mask & 2u) {
      element_slot = MMIO8(0x801eb6e0u + ((u32)(battler_index - 3u) * 0x118u));
      result += modifier_table[element_slot];
    }
    if (element_mask & 4u) {
      element_slot = MMIO8(0x801eb6e1u + ((u32)(battler_index - 3u) * 0x118u));
      result += modifier_table[element_slot];
    }
    if (element_mask & 8u) {
      element_slot = MMIO8(0x801eb6e2u + ((u32)(battler_index - 3u) * 0x118u));
      result += modifier_table[element_slot];
    }
    if (element_mask & 0x10u) {
      element_slot = MMIO8(0x801eb6e3u + ((u32)(battler_index - 3u) * 0x118u));
      result += modifier_table[element_slot];
    }
  }

  return result;
}
