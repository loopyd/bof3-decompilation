#include "bof3/bof3.h"

extern u8 magic10603RecordFlags[]; /* @source 0x801EF9F8 @kind data */

/* @source 0x801EF914
 * @behavior Reserves the first of 96 records in the pool at 0x801EF9F8 whose
 *           flags byte has bit 0 clear, sets that bit and returns its index.
 *           Records have stride 0x20. The original re-materializes the pool
 *           address for the flags load and again for the store, and returns
 *           0xFF once all 96 records are occupied. The caller at 0x801EEDB4
 *           masks the result to a byte and indexes the same pool with it to
 *           seed a record, and the reset at 0x801EECFC clears bytes 0 through
 *           2 of all 96 records, so a zero-extended byte result is what they
 *           consume. The pool ends at 0x801F05F8, the separate word global the
 *           rest of the overlay reads. Takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic10603_801EF914(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x60; index++) {
        offset = index * 0x20;
        flags = magic10603RecordFlags[offset];
        if (!(flags & 1)) {
            magic10603RecordFlags[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
