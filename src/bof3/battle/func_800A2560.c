#include "bof3/battle/battle15_internal.h"

/* @source 0x800A2560
 * @behavior Stores 4 into battle state byte D_80146375 and 0x4E into battle
 * state halfword D_801463C0, then calls func_8009E500.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800A2560(void) {
  D_80146375 = 4;
  D_801463C0 = 0x4E;
  func_8009E500();
}
