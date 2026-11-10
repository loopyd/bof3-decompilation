#include "bof3/bof3.h"

extern u8 D_801F0F1C[]; /* @source 0x801F0F1C @kind data */

/* @source 0x801F0DC0
 * @behavior Reserves the first of 24 records at 0x801F0F1C whose flags byte
 *           has bit 0 clear, sets that bit and returns its index. Records have
 *           stride 0x78. Returns 0xFF if all records are occupied. Takes no
 *           arguments and accesses only each record's flags byte. The caller
 *           at 0x801EF40C masks the result to a byte and tests for 0xFF before
 *           indexing the pool.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic08303_801F0DC0(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x18; index++) {
        offset = index * 0x78;
        flags = D_801F0F1C[offset];
        if (!(flags & 1)) {
            D_801F0F1C[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
