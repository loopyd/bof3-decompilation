#include "base/types.h"

extern u8 effectRecordTable[]; /* @source 0x801F0B38 @kind data */

/* @source 0x801F09C4
 * @behavior Reserves the first of 32 records at 0x801F0B38 whose flags byte
 *           has bit 0 clear, sets that bit and returns its index, or returns
 *           0xFF if every record is occupied. Records have stride 0x78.
 *           The caller at 0x801F009C masks the result to a byte and scales
 *           it by that stride. The reset at 0x801EED14 clears bytes 0 through
 *           2 of all 32 records. Takes no arguments and accesses only the
 *           leading flags byte of each record.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFreeRecordSlot(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x20; index++) {
        offset = index * 0x78;
        flags = effectRecordTable[offset];
        if (!(flags & 1)) {
            effectRecordTable[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
