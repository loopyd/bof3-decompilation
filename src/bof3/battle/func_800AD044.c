#include "bof3/battle/battle15_internal.h"

/* @behavior Tests bit (arg0 & 0x1F) of the bitmask word
 * D_80144F60[(arg0 & 0xFFFF) >> 5]; returns nonzero as a boolean.
 * @source 0x800AD044
 * @status exact
 * @match 100.00
 * @residual none
 */
u32 func_800AD044(u32 bitIndex) {
    u32 wordByteOffset;
    u32 mask;
    u32 word;

    bitIndex &= 0xFFFF;
    wordByteOffset = (bitIndex >> 5) * 4;
    mask = 1 << (bitIndex & 0x1F);
    word = *(u32 *)((u8 *)D_80144F60 + wordByteOffset);
    return (word & mask) != 0;
}
