#include "bof3/battle/battle15_internal.h"

/* @source 0x800A0E68
 * @behavior UNKNOWN: exact behavior is not yet documented.
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */

void func_800A0E68(void) {

  s32 half;

  half = func_801DC044(D_80146374, D_80146394, 0xFFFF) / 2;
  ((u16*)D_801463A0)[2] = half;
  if (half != 0) {
    querySelectionApplyInput(0x20);
  }
}
