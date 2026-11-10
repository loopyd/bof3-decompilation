#include "bof3/bof3.h"

extern u8 D_801F0604[]; /* @source 0x801F0604 @kind data */

/* @source 0x801F0524
 * @behavior Reserves the first of 64 records at 0x801F0604 whose flags byte
 * has bit 0 clear, sets that bit and returns the index. Records have stride
 * 0x78. Returns 0xFF when every record is occupied. Takes no arguments.
 * The caller at 0x801EF9D0 masks the result to a byte and uses that index
 * with the same stride to initialize the selected record.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic12503_801F0524(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x40; index++) {
        offset = index * 0x78;
        flags = D_801F0604[offset];
        if (!(flags & 1)) {
            D_801F0604[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
