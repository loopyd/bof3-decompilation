#include "bof3/battle/battle15_internal.h"

/* @source 0x8009FE10
 * @behavior Applies the 0x40 input mask to the active battler index byte D_80146394
 * through func_800A31E0, then raises bit 2 of byte +0x08 of the selection record
 * reachable through 0x801463A0 and stores -0x14 and -4 in its halfwords at +0x04 and
 * +0x06, reloading that pointer before each access.
 * @status exact
 * @match 100.00
 * @residual none
 */
void applyInputMask40ArmRecordField4Neg20Field6Neg4(void) {
  s16 *ptr;

  func_800A31E0(D_80146394, 0x40);
  ((u8 *)D_8014639C.record)[8] |= 2;
  ((s16 *)D_8014639C.record)[2] = -0x14;
  ptr = D_8014639C.record;
  ptr[3] = -4;
}
