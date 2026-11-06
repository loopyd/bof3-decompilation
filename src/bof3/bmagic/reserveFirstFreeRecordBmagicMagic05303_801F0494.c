#include "bof3/bof3.h"

extern u8 D_801F0578[]; /* @source 0x801F0578 @kind data */

/* @source 0x801F0494
 * @behavior Reserves the first of 32 records in the pool at 0x801F0578 whose
 *           flags byte has bit 0 clear, sets that bit and returns its index.
 *           Records have stride 0x78. The original re-materializes the pool
 *           address for the flags load and again for the store, and returns
 *           0xFF once all 32 records are occupied. The caller at 0x801EF4E8
 *           compares the zero-extended result against 0xFF and indexes the
 *           same pool with it, so a byte-wide index is what it consumes.
 *           Takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic05303_801F0494(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x20; index++) {
        offset = index * 0x78;
        flags = D_801F0578[offset];
        if (!(flags & 1)) {
            D_801F0578[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
