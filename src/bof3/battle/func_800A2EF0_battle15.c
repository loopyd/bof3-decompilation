#include "bof3/battle/battle15_internal.h"

/* @behavior Returns the signed element modifier selected by the input mask bits
 *   0x40/0x80/0x100 for the battler chosen by the index argument: the mask bit
 *   picks the element byte of the 320-byte-stride record above 0x80145F35
 *   (indices below three) or of the 280-byte-stride enemy record above
 *   0x801EB6E5 (larger indices), and that byte selects the local s16 modifier
 *   table at 0x800B495C. Zero when no mask bit is set.
 * @source 0x800A2EF0
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_800A2EF0(u8 battler_index, u32 flags)
{
  s32 result = 0;
  u32 offset;

  if (battler_index < 3u) {
    if (flags & 0x40u) {
      result = D_800B495C[D_80145F35[battler_index * 0x140u]];
    }
    if (flags & 0x80u) {
      result = D_800B495C[D_80145F36[battler_index * 0x140u]];
    }
    if (flags & 0x100u) {
      result = D_800B495C[D_80145F37[battler_index * 0x140u]];
    }
  } else {
    if (flags & 0x40u) {
      offset = (battler_index - 3) * 0x118;
      result = D_800B495C[D_801EB6E5[offset]];
    }
    if (flags & 0x80u) {
      offset = (battler_index - 3) * 0x118;
      result = D_800B495C[D_801EB6E6[offset]];
    }
    if (flags & 0x100u) {
      offset = (battler_index - 3) * 0x118;
      result = D_800B495C[D_801EB6E7[offset]];
    }
  }
  return result;
}
