#include "internal.h"

void func_801DFFEC(void)
{
    u8 *work;
    u8 *next;
    s32 x;
    s32 y;

    work = D_1F800044;
    x = *(s32 *)(work + 0x34);
    work[9] = 8;
    y = *(s32 *)(work + 0x38);
    *(u32 *)(work + 0x0C) = (u32)((u32)((x & ~0x7FFF) - x) >> 3);
    next = D_1F800044;
    *(u32 *)(work + 0x10) = (u32)((u32)((y & ~0x7FFF) - y) >> 3);
    next[2] = (u8)(next[2] + 1);
}
