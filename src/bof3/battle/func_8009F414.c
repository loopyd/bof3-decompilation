#include "bof3/battle/battle15_internal.h"

/* @source 0x8009F414
 * @behavior Stores 4 into battle state byte D_80146375 and 0x5D into battle
 * state halfword D_801463C0, then calls func_8009DCC8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009F414(void) {
  D_80146375 = 4;
  D_801463C0 = 0x5D;
  func_8009DCC8();
}
