#include "bof3/bof3.h"

extern u8 magic13103RecordFlags[]; /* @source 0x801F1CA0 @kind data */

/* @source 0x801F1A58
 * @behavior Reserves the first of 48 records in the pool at 0x801F1CA0 whose
 *           flags byte has bit 0 clear, sets that bit and returns its index.
 *           Records have stride 0x2C. The original re-materializes the pool
 *           address for the flags load and again for the store, and returns
 *           0xFF once all 48 records are occupied. The callers at 0x801EFD78
 *           and 0x801EFE04 compare the returned byte against 0xFF and use the
 *           index to clear per-record byte fields, so a zero-extended byte
 *           result is what they consume. Takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic13103_801F1A58(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x30; index++) {
        offset = index * 0x2C;
        flags = magic13103RecordFlags[offset];
        if (!(flags & 1)) {
            magic13103RecordFlags[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
