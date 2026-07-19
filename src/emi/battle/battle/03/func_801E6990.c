#include "bof3/bof3.h"

void func_801E6990(void) {
  *(volatile u8*)(*(volatile void**)SPAD_ADDRESS(0x44u) + 9) = 0;
  *(volatile u8*)(*(volatile void**)SPAD_ADDRESS(0x44u) + 1) += 1;
}
