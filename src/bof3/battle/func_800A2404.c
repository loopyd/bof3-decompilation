#include "bof3/battle/battle15_internal.h"

/* @source 0x800A2404
 * @behavior Stores 4 into battle state byte D_80146375 and 0x6A into battle
 * state halfword D_801463C0, then calls func_800A0378.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800A2404(void) {
  D_80146375 = 4;
  D_801463C0 = 0x6A;
  func_800A0378();
}
