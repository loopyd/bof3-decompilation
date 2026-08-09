#include "bof3/battle/battle15_internal.h"

/*
 * @source 0x800A581C
 * @behavior Calls func_8015DF18(0x104), sets D_80148627 to 2, sets bit 2 in local work field unk_128, and increments D_801462E4.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void setupMode104ArmWorkBit2(void) {
  u8* counter;

  func_8015DF18(0x104);
  D_80148627 = 2;
  D_801EBF08_PTR->unk_128 |= 4;
  counter = &D_801462E4;
  *counter = *counter + 1;
}
