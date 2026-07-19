#include "bof3/bof3.h"

void func_801E3BAC(void) {
  *(volatile u32*)SPAD_ADDRESS(0x44u) = 0;
  *(volatile u8*)(*(volatile void**)SPAD_ADDRESS(0x44u) + 2) = 1;
}
