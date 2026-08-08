#include "internal.h"

/**
 * @source 0x80097D58
 * @behavior Configure the active selection slot from its selection-kind flags.
 */
void func_80097D58(void)
{
    u8 *slot;
    u16 kind;

    slot = D_801EB4D8;
    kind = *(u16 *)(slot + 2);
    if ((D_801CA718[kind].flags & 0x20) != 0) {
        slot[0] = 0x40;
    } else {
        slot[0] = 0x80;
    }
    D_801462E4 = 1;
    D_80145AC8 = 0;
    D_801462EF = 1;
}
