#include "bof3/bof3.h"

/* Record pool of this overlay: 24 records of stride 0x78 starting at
 * 0x801F0694, so the pool ends at 0x801F11D4, the last byte of the segment.
 * Every access here and in the sweep at 0x801EEC68 re-materializes the pool
 * address (lui $at,%hi / addu $at,$at,index*0x78 / lbu|sb %lo) instead of
 * keeping a base in a register. The framework symbol name is kept because the
 * still-assembled sweep and the three callers reach the pool through it. */
extern u8 D_801F0694[]; /* @source 0x801F0694 @kind data */

/* @source 0x801F0490
 * @behavior Reserves the first of the 24 records in the pool at 0x801F0694
 *           whose flags byte has bit 0 clear, sets that bit and returns its
 *           index. Records have stride 0x78, so the pool spans 0x801F0694 to
 *           0x801F11D4. The original re-materializes the pool address for the
 *           flags load and again for the store, and returns 0xFF once every
 *           record is occupied. Its three callers, at 0x801EEDC8, 0x801EEE68
 *           and 0x801EEF08, mask the result to a byte and index the same pool
 *           with it to fill record bytes 0x1, 0x9 and 0xB; the sweep at
 *           0x801EEC68 walks the same 24 records and treats that bit 0 as the
 *           in-use flag. Takes no arguments and reads or writes no state
 *           beyond the flags byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic05903_801F0490(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x18; index++) {
        offset = index * 0x78;
        flags = D_801F0694[offset];
        if (!(flags & 1)) {
            D_801F0694[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
