#include "bof3/battle/battle15_internal.h"

/* @source 0x800A0E68
 * @behavior UNKNOWN: exact behavior is not yet documented.
 * @status exact
 * @match 100.00
 * @residual none
 */

void func_800A0E68(void) {

  s32 half;

  half = func_801DC044(D_80146374, D_80146394, 0xFFFF) / 2;
  ((u16*)D_801463A0)[2] = half;
  if (half != 0) {
    querySelectionApplyInput(0x20);
  }
}
