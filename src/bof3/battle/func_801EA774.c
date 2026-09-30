#include "bof3/battle/battle03_internal.h"

/* @source 0x801EA774
 * @behavior Reads the panel halfword +0x06 of the state object published at
 * 0x80148648: when it reads -0x16, the dispatched-handler mask byte at
 * 0x80146329 has the bits of the +0x01 byte of the dispatch slot published at
 * 0x801EB4DC cleared from it and func_80158E20 is called; every other value
 * rewrites that same halfword 8 lower.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801EA774(void) {
  u8* mask;
  s16 state;

  state = *(s16*)&D_80148648[6];
  if (state == -0x16) {
    mask = &D_80146329;
    *mask &= (u8)~D_801EB4DC[1];
    func_80158E20();
  } else {
    *(s16*)&D_80148648[6] = state - 8;
  }
}
