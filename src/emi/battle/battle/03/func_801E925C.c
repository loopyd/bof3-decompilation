#include "bof3/bof3.h"

extern u32 D_80148648;

void func_801E925C(void) {
  *(volatile u8*)(D_80148648 + 3) += 1;
}
