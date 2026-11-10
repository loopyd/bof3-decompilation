#include "base/types.h"

extern u8 D_801EFCA4[]; /* @source 0x801EFCA4 @kind data */

/* @source 0x801EFC24
 * @behavior Reserves the first of 64 records in the pool at 0x801EFCA4 whose
 *           flags byte has bit 0 clear, sets that bit and returns the index,
 *           or returns 0xFF when every record is occupied. Records have stride
 *           0x78. The caller at 0x801EEE00 masks the result to a byte and uses
 *           that stride to fill the selected record. Takes no arguments and
 *           changes no state beyond the selected flags byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic04303_801EFC24(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x40; index++) {
        offset = index * 0x78;
        flags = D_801EFCA4[offset];
        if (!(flags & 1)) {
            D_801EFCA4[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
