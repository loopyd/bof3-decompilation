#include "bof3/bof3.h"

/* Record pool of this overlay at 0x801F0424: 80 records of stride 0x78 whose
 * byte 0 holds the flag bit this body tests and sets, and whose bytes 0x00 to
 * 0x02 entry stage 0 clears for every record. The map carries no row at this
 * address and the pool base is generated from the name the payload's own asm
 * used, so the flags byte stays address-anchored. */
extern u8 D_801F0424[]; /* @source 0x801F0424 @kind data */

/* @source 0x801F0368
 * @behavior Reserves the first of the 80 records of the 0x78-byte-stride pool
 *           at 0x801F0424 whose flags byte has bit 0 clear: it sets that bit
 *           and returns the record index. It returns 0xFF when every record is
 *           occupied. The body takes no arguments, keeps no saved register and
 *           no frame, and touches only the flags byte of each record, computing
 *           the stride from the byte-masked index and testing the incremented
 *           index against 0x50 at the loop bottom. The one jal to it, in the
 *           step-2 handler at 0x801EF2CC, masks the result to a byte, tests it
 *           against 0xFF and indexes the same stride-0x78 pool with it. The
 *           roles of the pool's other bytes are not established here.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic16903_801F0368(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x50; index++) {
        offset = index * 0x78;
        flags = D_801F0424[offset];
        if (!(flags & 1)) {
            D_801F0424[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
