#include "bof3/battle/battle15_internal.h"

/* @behavior Returns the signed element modifier selected by the input mask bits
 *   0x40/0x80/0x100 for the battler chosen by the index argument: the mask bit
 *   picks the element byte of the 320-byte-stride record above 0x80145F35
 *   (indices below three) or of the 280-byte-stride enemy record above
 *   0x801EB6E5 (larger indices), and that byte selects the local s16 modifier
 *   table at 0x800B494C. The halfword mask 0xFFFF sentinel is returned when no
 *   element bit is set.
 * @source 0x800A2D70
 * @status exact
 * @match 100.00
 * @residual none
 */
s16 func_800A2D70(s32 battler_index, s32 selection_mask)
{
  s32 result = 0;
  u16 mask = selection_mask;
  u32 offset;

  battler_index &= 0xFF;

  if ((u32)battler_index < 3u) {
    if (selection_mask & 0x40) {
      result = D_800B494C[D_80145F35[(u32)battler_index * 0x140u]];
    }
    if (selection_mask & 0x80) {
      result = D_800B494C[D_80145F36[(u32)battler_index * 0x140u]];
    }
    if (selection_mask & 0x100) {
      result = D_800B494C[D_80145F37[(u32)battler_index * 0x140u]];
    }
  } else {
    if (selection_mask & 0x40) {
      offset = (u32)(battler_index - 3) * 0x118u;
      result = D_800B494C[D_801EB6E5[offset]];
    }
    if (selection_mask & 0x80) {
      offset = (u32)(battler_index - 3) * 0x118u;
      result = D_800B494C[D_801EB6E6[offset]];
    }
    if (selection_mask & 0x100) {
      offset = (u32)(battler_index - 3) * 0x118u;
      result = D_800B494C[D_801EB6E7[offset]];
    }
  }

  if ((mask & 0x1c0) == 0) {
    result = 0xFFFF;
  }

  return result;
}
