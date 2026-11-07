#include "bof3/bof3.h"

extern u8 magic09303RecordFlags[]; /* @source 0x801F08D4 @kind data */

/* @source 0x801F05BC
 * @behavior Reserves the first of the 32 records in the pool at 0x801F08D4
 *           whose flags byte has bit 0 clear, sets that bit and returns the
 *           index, or returns 0xFF once every record is occupied. Records have
 *           stride 0x78. The original re-materializes the pool address for the
 *           flags load and again for the store. Both callers, at 0x801EEEE4 and
 *           0x801EEF54, mask the result to a byte and index the same pool with
 *           it to fill record byte 0x1, record byte 0x4 and record word 0x74,
 *           and the sweep at 0x801EEC64 walks the same 32 records treating that
 *           bit 0 as the in-use flag. Takes no arguments and reads or writes no
 *           state beyond the flags byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic09303_801F05BC(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x20; index++) {
        offset = index * 0x78;
        flags = magic09303RecordFlags[offset];
        if (!(flags & 1)) {
            magic09303RecordFlags[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
