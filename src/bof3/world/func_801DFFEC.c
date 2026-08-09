#include "bof3/world/area03004_internal.h"

/* @behavior computes signed step values from the scratch work-record
 * x/y coordinates at offsets 0x34/0x38, stores them at offsets 0x0C/0x10,
 * sets work byte +0x09 to 8, and increments work byte +0x02.
 * @source 0x801DFFEC
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
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
