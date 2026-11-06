#include "base/types.h"

extern u8 magic08103RecordFlags[]; /* @source 0x801EFA8C @kind data */

/* @source 0x801EFA20
 * @behavior Reserves the first of 48 records in the pool at 0x801EFA8C whose
 *           flags byte has bit 0 clear, sets that bit and returns its index.
 *           Records have stride 0x78. The original re-materializes the pool
 *           address for the flags load and again for the store, and returns
 *           0xFF once all 48 records are occupied. The caller at 0x801EF1A8
 *           masks the result to a byte and indexes the same pool with it to
 *           fill record word 0x74 and bytes 9 through 0xB, and the sweep at
 *           0x801EEC30 treats that same bit 0 as the in-use flag, so a
 *           zero-extended byte result is what they consume. The reset at
 *           0x801EED44 clears bytes 0 through 2 of all 48 records. Takes no
 *           arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic08103_801EFA20(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x30; index++) {
        offset = index * 0x78;
        flags = magic08103RecordFlags[offset];
        if (!(flags & 1)) {
            magic08103RecordFlags[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
