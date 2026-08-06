#include "internal.h"

/* @source 0x8009C87C */
/* @behavior Sets or clears bit `bit` in byte offset 0xE1 according to nonzero `set`. */
void setEntryBitE1(u8 *arg0, s32 bit, u8 set) {
    u8 mask;

    mask = 1 << bit;
    if (set != 0) {
        arg0[0xE1] |= mask;
    } else {
        arg0[0xE1] &= ~mask;
    }
}
