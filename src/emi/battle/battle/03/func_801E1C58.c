#include "bof3/bof3.h"

void func_801E1C58(void) {
    *(volatile u8 *)(*(volatile void **)SPAD_ADDRESS(0x44u) + 1) = 2;
    *(volatile u8 *)(*(volatile void **)SPAD_ADDRESS(0x44u) + 2) = 0;
}
