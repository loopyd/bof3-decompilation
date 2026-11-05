#include "bof3/battle/battle15_internal.h"

/* @source 0x800A403C
 * @behavior Calls func_800A3F28 and tests the current selection roll. On
 *   acceptance, clamps indexed selection-record byte +0x14 using the signed
 *   modifier from the current 20-byte table entry, calls func_800A9BD8 with
 *   the current battler byte, and returns 0; otherwise returns 1.
 * @status partial
 * @match 83.78
 * @residual Table-index and scaled-offset registers v0/v1 are exchanged;
 *   31/37 instructions match, with the original 148-byte size and frame.
 */
s32 func_800A403C(u8 index)
{
  s32 result;

  func_800A3F28();
  if ((u8)func_800A3A10(D_80146374, D_80146394) == 0) {
    clampIndexedBattleByte14((s8)D_801CA71B[D_801463C0 * 20], index);
    func_800A9BD8(D_80146394);
    result = 0;
  } else {
    result = 1;
  }
  return result;
}
