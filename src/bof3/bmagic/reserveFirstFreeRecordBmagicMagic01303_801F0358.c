#include "base/types.h"

extern u8 magic01303RecordFlags[]; /* @source 0x801F03E4 @kind data */

/* @source 0x801F0358
 * @behavior Reserves the first of the 64 records in the pool at 0x801F03E4
 *           whose flags byte has bit 0 clear, sets that bit and returns the
 *           index, or returns 0xFF once every record is occupied. Records have
 *           stride 0x78 and the pool runs to the end of the payload. The
 *           caller loop at 0x801EF248 masks the result to a byte and compares
 *           it with 0xFF before indexing the same pool, the sweep at
 *           0x801EEC88 treats bit 0 as the in-use flag and the reset at
 *           0x801EED50 clears bytes 0 through 2 of every record. Takes no
 *           arguments and reads and writes no state beyond the flags byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic01303_801F0358(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x40; index++) {
        offset = index * 0x78;
        flags = magic01303RecordFlags[offset];
        if (!(flags & 1)) {
            magic01303RecordFlags[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
