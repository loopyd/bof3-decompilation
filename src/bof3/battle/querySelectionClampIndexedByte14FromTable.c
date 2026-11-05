#include "bof3/battle/battle15_internal.h"

/* @source 0x800A40D0
 * @behavior Tests the current selection roll and returns 1 on rejection.
 *   Otherwise uses the signed byte at D_801CA71B plus the current table index
 *   times 20 as the delta for clampIndexedBattleByte14 at the low-byte index,
 *   calls func_800A9BD8 with the current battler byte, and returns 0.
 * @status partial
 * @match 81.08
 * @residual Table-index and scaled-offset registers are exchanged; an extra
 *   low-byte return mask remains.
 */
u8 querySelectionClampIndexedByte14FromTable(u8 index)
{
  u8 result;

  result = func_800A3A10(D_80146374, D_80146394);
  if (result == 0) {
    clampIndexedBattleByte14((s8)D_801CA71B[D_801463C0 * 20], index);
    func_800A9BD8(D_80146394);
    result = 0;
  } else {
    result = 1;
  }
  return result;
}
