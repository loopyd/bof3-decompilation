#include "bof3/bof3.h"

void func_801DDF00(void) {
  *(volatile u8*)(*(volatile void**)SPAD_ADDRESS(0x44u) + 0x90) = 5;
  *(volatile u8*)(*(volatile void**)SPAD_ADDRESS(0x44u) + 0x91) = 2;
  *(volatile u8*)(*(volatile void**)SPAD_ADDRESS(0x44u) + 0x92) = 0;
}
