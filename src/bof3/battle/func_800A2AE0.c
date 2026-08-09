#include "bof3/battle/battle15_internal.h"

/* @behavior sums elemental resistance modifiers for a battler based on a bitmask
 * of active elements. each element slot in the battler's data holds an index
 * into a s16 modifier table at 0x800b493c.
 * @source 0x800A2AE0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
s16 func_800A2AE0(u8 battler_index, u16 element_mask) {
  s32 result;
  u32 offset;
  u32 offset1;

  result = 0;

  if (battler_index < 3u) {
    if (element_mask & 1u) {
      result += D_800B493C[D_80145F2F[battler_index * 0x140u]];
    }
    if (element_mask & 2u) {
      result += D_800B493C[D_80145F30[battler_index * 0x140u]];
    }
    if (element_mask & 4u) {
      result += D_800B493C[D_80145F31[battler_index * 0x140u]];
    }
    if (element_mask & 8u) {
      result += D_800B493C[D_80145F32[battler_index * 0x140u]];
    }
    if (element_mask & 0x10u) {
      result += D_800B493C[D_80145F33[battler_index * 0x140u]];
    }
  } else {
    if (element_mask & 1u) {
      offset1 = (battler_index - 3) * 0x118;
      result += D_800B493C[D_801EB6DF[offset1]];
    }
    if (element_mask & 2u) {
      offset = (battler_index - 3) * 0x118;
      result += D_800B493C[D_801EB6E0[offset]];
    }
    if (element_mask & 4u) {
      offset = (battler_index - 3) * 0x118;
      result += D_800B493C[D_801EB6E1[offset]];
    }
    if (element_mask & 8u) {
      offset = (battler_index - 3) * 0x118;
      result += D_800B493C[D_801EB6E2[offset]];
    }
    if (element_mask & 0x10u) {
      offset = (battler_index - 3) * 0x118;
      result += D_800B493C[D_801EB6E3[offset]];
    }
  }

  return result;
}
