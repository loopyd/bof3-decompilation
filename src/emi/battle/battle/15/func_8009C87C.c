#include "internal.h"

void func_8009C87C(u8 *arg0, s32 bit, u8 set) {
    u8 mask;

    mask = 1 << bit;
    if (set != 0) {
        arg0[0xE1] |= mask;
    } else {
        arg0[0xE1] &= ~mask;
    }
}
