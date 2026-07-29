#include "internal.h"

void func_80098254(void) {
  u8 counter;

  func_8015DF18(0x104);
  D_801462EF = 0;
  BATTLE_ACTIVE_SELECTION_SLOT_PTR[1] = 0;
  *(u32*)(BATTLE_ACTIVE_SELECTION_SLOT_PTR + 0xC) |= 1;
  D_801EBF08_PTR->unk_01 = 2;
  counter = D_80146303;
  D_801462E1 = 1;
  D_801462E2 = 0;
  D_801462E3 = 0;
  D_80146303 = counter + 1;
}
