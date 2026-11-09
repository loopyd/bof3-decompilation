#include "bof3/bof3.h"

/* Base of the 48-record pool this overlay keeps in its own image, right after
 * the fifteen-word handler-pointer run at 0x801F07BC. Splat binds the address
 * through this target's generated undefined_syms_auto.txt, so no target-map
 * row is needed. The caller at 0x801EF0CC indexes the same cell with the same
 * 0x78 stride. */
extern u8 D_801F07F8[]; /* @source 0x801F07F8 @kind data */

/* @source 0x801F0758
 * @behavior Reserves the first of 48 records in the pool at 0x801F07F8 whose
 *           flags byte has bit 0 clear, sets that bit and returns its index.
 *           Records have stride 0x78 (sll 4, subu, sll 3). The original
 *           re-materializes the pool address for the flags load and again for
 *           the store, tests the loaded byte with andi 0x1, and returns 0xFF
 *           once all 48 records are occupied, the scan bound being the sltiu
 *           0x30 on the index masked to a byte. Its only in-image caller at
 *           0x801EF0CC calls it three times, masks the result to a byte,
 *           compares that against 0xFF and indexes the same pool with the same
 *           0x78 stride to fill record bytes 1 through 3 and word 0x74, so a
 *           zero-extended byte result is what the caller consumes. Leaf with
 *           no frame that takes no arguments and returns the index or 0xFF
 *           through the jr $ra at 0x801F07B4. This 0x64-byte body matches
 *           0x801EFA20 in emi/bmagic/magic081/03 word for word apart from the
 *           pool address words and that body's own branch target word, so the
 *           magic081 lift is shape evidence rather than a clone source.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic09203_801F0758(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x30; index++) {
        offset = index * 0x78;
        flags = D_801F07F8[offset];
        if (!(flags & 1)) {
            D_801F07F8[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
