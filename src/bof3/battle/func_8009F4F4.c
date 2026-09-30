#include "bof3/battle/battle15_internal.h"

/* @source 0x8009F4F4
 * @behavior Stores 4 into battle state byte D_80146375 and 0x52 into battle
 * state halfword D_801463C0, then calls func_800A403C with argument 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009F4F4(void) {
  D_80146375 = 4;
  D_801463C0 = 0x52;
  func_800A403C(1);
}
