#include "bof3/bof3.h"

extern u8 magic10503RecordFlags[]; /* @source 0x801F00A0 @kind data */

/* @source 0x801EFC8C
 * @behavior Reserves the first of 64 records at 0x801F00A0 whose flags byte
 *           has bit 0 clear, sets that bit and returns its index, or returns
 *           0xFF when all records are occupied. Records have stride 0x78.
 *           The caller at 0x801EF124 masks the result to a byte, checks 0xFF,
 *           and uses the index to initialize the same record pool. Takes no
 *           arguments and changes only the selected flags byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic10503_801EFC8C(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x40; index++) {
        offset = index * 0x78;
        flags = magic10503RecordFlags[offset];
        if (!(flags & 1)) {
            magic10503RecordFlags[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
