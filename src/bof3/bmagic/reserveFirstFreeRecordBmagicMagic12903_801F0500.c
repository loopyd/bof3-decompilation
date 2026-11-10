#include "bof3/bof3.h"

extern u8 magic12903RecordFlags[]; /* @source 0x801F05D8 @kind data */

/* @source 0x801F0500
 * @behavior Reserves the first of the 64 records at 0x801F05D8 whose flags
 *           byte has bit 0 clear, sets that bit and returns the index, or
 *           returns 0xFF when every record is occupied. Records have stride
 *           0x78. Takes no arguments and accesses only the flags bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic12903_801F0500(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x40; index++) {
        offset = index * 0x78;
        flags = magic12903RecordFlags[offset];
        if (!(flags & 1)) {
            magic12903RecordFlags[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
