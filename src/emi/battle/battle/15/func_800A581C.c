#include "internal.h"

void func_800A581C(void) {
  u8* counter;

  func_8015DF18(0x104);
  D_80148627 = 2;
  D_801EBF08_PTR->unk_128 |= 4;
  counter = &D_801462E4;
  *counter = *counter + 1;
}
