#include "base/types.h"

extern u8 magic21303RecordFlags[]; /* @source 0x801EF8C0 @kind data */

/* @source 0x801EF7EC
 * @behavior Reserves the first of the 64 records in the pool at 0x801EF8C0
 *           whose flags byte has bit 0 clear, sets that bit and returns the
 *           index, or returns 0xFF once every record is occupied. Records have
 *           stride 0x78, so the pool covers 0x1E00 bytes from 0x801EF8C0 to
 *           the end of the payload at 0x801F16C0, starting 0x20 bytes into the
 *           data blob T_801EF8A0. The pool address is re-materialized for the
 *           flags load and again for the store. The sweep at 0x801EEC80 tests
 *           that same bit 0 as the in-use flag before dispatching a record, and
 *           the reset at 0x801EED34 clears bytes 0 through 2 of all 64 records.
 *           Takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic21303_801EF7EC(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x40; index++) {
        offset = index * 0x78;
        flags = magic21303RecordFlags[offset];
        if (!(flags & 1)) {
            magic21303RecordFlags[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
