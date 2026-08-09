#include "bof3/battle/battle15_internal.h"

/* @behavior Tests bit (arg0 & 0x1F) of the bitmask word
 * D_80144F60[(arg0 & 0xFFFF) >> 5]; returns nonzero as a boolean.
 * @source 0x800AD044
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u32 func_800AD044(u32 arg0) {
    u32 idx;
    u32 mask;
    u32 word;

    arg0 &= 0xFFFF;
    idx = (arg0 >> 5) * 4;
    mask = 1 << (arg0 & 0x1F);
    word = *(u32 *)((u8 *)D_80144F60 + idx);
    return (word & mask) != 0;
}
