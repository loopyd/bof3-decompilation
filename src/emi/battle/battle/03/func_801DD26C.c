#include "bof3/bof3.h"
#include "internal.h"

void func_801DD26C(s8 arg0) {
    BATTLE_GLOBAL_BYTE_6322 -= 1;
    *(volatile u8 *)(D_8014630C + (BATTLE_GLOBAL_BYTE_6322 & 0xFF)) = arg0;
}
