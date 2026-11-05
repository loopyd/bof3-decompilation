#include "bof3/battle/battle15_internal.h"

/* @source 0x800A4160
 * @behavior Tests the current selection roll and returns 1 when it rejects the
 *   operation. Otherwise adds the signed low byte of delta to byte +0x14 of
 *   the selection record at the low-byte index, clamps it through
 *   clampIndexedBattleByte14, calls func_800A9BD8 with the current battler
 *   byte D_80146394, and returns 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 querySelectionClampIndexedByte14(u8 index, s8 delta)
{
  if ((u8)func_800A3A10(D_80146374, D_80146394) == 0) {
    clampIndexedBattleByte14(delta, index);
    func_800A9BD8(D_80146394);
    return 0;
  }
  return 1;
}
