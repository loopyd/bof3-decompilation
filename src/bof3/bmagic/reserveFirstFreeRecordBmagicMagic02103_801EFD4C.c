#include "base/types.h"

extern u8 magic02103RecordFlags[]; /* @source 0x801EFE14 @kind data */

/* @source 0x801EFD4C
 * @behavior Reserves the first of 64 records in the pool at 0x801EFE14 whose
 *           flags byte has bit 0 clear, sets that bit and returns the index,
 *           or returns 0xFF when every record is occupied. Records have stride
 *           0x78. The caller at 0x801EF164 masks the result to a byte and uses
 *           that stride to fill the selected record. Takes no arguments and
 *           changes no state beyond the selected flags byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic02103_801EFD4C(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x40; index++) {
        offset = index * 0x78;
        flags = magic02103RecordFlags[offset];
        if (!(flags & 1)) {
            magic02103RecordFlags[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
