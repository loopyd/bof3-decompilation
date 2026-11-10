#include "bof3/bof3.h"

extern u8 D_801F0900[]; /* @source 0x801F0900 @kind data */

/* @source 0x801F0828
 * @behavior Reserves the first of the 64 records at 0x801F0900 whose flags
 *           byte has bit 0 clear, sets that bit and returns the index, or
 *           returns 0xFF when every record is occupied. Records have stride
 *           0x78. Takes no arguments and accesses only the flags bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic12403_801F0828(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x40; index++) {
        offset = index * 0x78;
        flags = D_801F0900[offset];
        if (!(flags & 1)) {
            D_801F0900[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
