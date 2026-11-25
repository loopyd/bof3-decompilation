#include "bof3/bof3.h"

extern u8 recordFlags[]; /* @source 0x801F05D8 @kind data */

/* @source 0x801F0500
 * @behavior Reserves the first of the 64 records at 0x801F05D8 whose flags
 *           byte has bit 0 clear, sets that bit and returns the index, or
 *           returns 0xFF when every record is occupied. Records have stride
 *           0x78. Takes no arguments and accesses only the flags bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 resourceReserveFreeRecord11(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x40; index++) {
        offset = index * 0x78;
        flags = recordFlags[offset];
        if (!(flags & 1)) {
            recordFlags[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
