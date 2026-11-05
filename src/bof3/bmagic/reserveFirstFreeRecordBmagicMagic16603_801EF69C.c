#include "base/types.h"

extern u8 magic16603RecordFlags[]; /* @source 0x801EF724 @kind data */

/* @source 0x801EF69C
 * @behavior Reserves the first of 64 records whose flags byte has bit 0 clear,
 *           sets that bit, and returns its index. Records have stride 0x78;
 *           returns 0xFF if all records are occupied.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic16603_801EF69C(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x40; index++) {
        offset = index * 0x78;
        flags = magic16603RecordFlags[offset];
        if (!(flags & 1)) {
            magic16603RecordFlags[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
