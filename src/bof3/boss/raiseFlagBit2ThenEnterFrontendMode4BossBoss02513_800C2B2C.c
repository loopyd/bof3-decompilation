#include "bof3/bof3.h"

extern u16 D_801462E8;

/* @source 0x800C2B2C
 * @behavior Raises bit 0x2 of the shared battle flag halfword at 0x801462E8,
 * then requests frontend mode 4 through func_8014ECAC (FrontendSetMode); it
 * reads and writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlagBit2ThenEnterFrontendMode4BossBoss02513_800C2B2C(void) {
  u16* flags;

  flags = &D_801462E8;
  *flags |= 2;
  func_8014ECAC(4);
}
