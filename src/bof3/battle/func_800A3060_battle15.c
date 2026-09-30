#include "bof3/battle/battle15_internal.h"

/* @behavior Returns the signed battle modifier selected by the byte index
 * argument when input mask bit 0x20 is set, and zero otherwise. Indices below
 * three read the 320-byte-stride table at 0x80145F34; larger indices read the
 * 280-byte-stride table at 0x801EB6E4; the resulting byte selects the local
 * s16 modifier table at 0x800B496C.
 * @source 0x800A3060
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_800A3060(u8 selector, u32 flags)
{
  s32 result = 0;
  u32 offset;

  if (selector < 3) {
    if (flags & 0x20) {
      result = D_800B496C[D_80145F34[selector * 320]];
    }
  } else {
    if (flags & 0x20) {
      offset = (selector - 3) * 280;
      result = D_800B496C[D_801EB6E4[offset]];
    }
  }
  return result;
}
