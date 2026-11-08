#include "bof3/bof3.h"

extern u16 D_801462E8;

/* @source 0x800C1C3C
 * @behavior Raises bit 0x2 of the shared battle flag halfword at 0x801462E8,
 * then requests frontend mode 4 through func_8014ECAC (FrontendSetMode); it
 * reads and writes no other state. This overlay installs the function into the
 * second shared battle hook slot (0x801463A8) from
 * publishBattleHooksBossBoss03516_800C1B4C, so the battle step reaches it by
 * jalr through that slot; no in-payload jal targets it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlagBit2ThenEnterFrontendMode4BossBoss03516_800C1C3C(void) {
  u16* flags;

  flags = &D_801462E8;
  *flags |= 2;
  func_8014ECAC(4);
}
