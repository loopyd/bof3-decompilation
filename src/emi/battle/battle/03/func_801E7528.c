#include "bof3/bof3.h"

extern u8 D_801EB4E0;

void func_801E7528(void) {
  *(volatile u8*)(D_801EB4E0 + 0x48) = 2;
  *(volatile u8*)(*(volatile void**)SPAD_ADDRESS(0x44u) + 1) += 1;
}
