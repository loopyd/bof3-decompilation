#include "bof3/bof3.h"

/* Base of the 0xA0-record pool this overlay keeps in its own image. The pool
 * occupies 0x801F0670 through 0x801F21F0, the last 0x1B80 bytes of the payload
 * at stride 0x2C, and ends exactly at the pointer cell the overlay publishes
 * its stepped work record through. Splat binds the address through this
 * target's generated undefined_syms_auto.txt, so no target-map row is needed.
 * The caller at 0x801EF1A8 indexes the same cell with the same 0x2C stride. */
extern u8 D_801F0670[]; /* @source 0x801F0670 @kind data */

/* @source 0x801F03E8
 * @behavior Reserves the first of 0xA0 records in the pool at 0x801F0670 whose
 *           flags byte has bit 0 clear, sets that bit and returns its index.
 *           Records have stride 0x2C. The original re-materializes the pool
 *           address for the flags load and again for the store, tests the
 *           loaded byte with andi 0x1, and returns 0xFF once all 0xA0 records
 *           are occupied, the scan bound being the sltiu 0xA0 on the index
 *           masked to a byte. Its only in-image caller, at 0x801EF1A8, calls it
 *           in a spawn loop, masks the result to a byte, compares that against
 *           0xFF and indexes the same pool with the same 0x2C stride, so a
 *           zero-extended byte result is what the caller consumes. Leaf with no
 *           frame that takes no arguments and returns the index or 0xFF through
 *           the jr $ra at 0x801F0450.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic07303_801F03E8(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0xA0; index++) {
        offset = index * 0x2C;
        flags = D_801F0670[offset];
        if (!(flags & 1)) {
            D_801F0670[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
