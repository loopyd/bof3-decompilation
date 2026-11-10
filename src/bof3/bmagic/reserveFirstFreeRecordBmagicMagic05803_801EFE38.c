#include "bof3/bof3.h"

/* Record pool of this overlay: 36 records of stride 0x78 starting at
 * 0x801EFEF4, so the pool ends at 0x801F0FD4, the byte table D_801F0FD4 the
 * reset at 0x801EED30 clears after the pool. The whole span is zero-filled in
 * the payload image. Every access re-materializes the pool address - lui
 * $at,%hi / addu $at,$at,index*0x78 / lbu|sb %lo - instead of keeping a base in
 * a register, and the still-assembled sweep at 0x801EEC90 and reset at
 * 0x801EED40 reach the same pool through the generated
 * undefined_syms_auto.txt row, so that framework name is kept here. Record
 * byte 0 bit 0 is the in-use flag the sweep tests, and record word 0x74
 * (0x801EFF68) is the battle-record pointer the sweep publishes through
 * 0x1F800044. */
extern u8 D_801EFEF4[]; /* @source 0x801EFEF4 @kind data */

/* @source 0x801EFE38
 * @behavior Reserves the first of the 36 records in the pool at 0x801EFEF4
 *           whose flags byte has bit 0 clear, sets that bit and returns its
 *           index. Records have stride 0x78, so the pool spans 0x801EFEF4 to
 *           0x801F0FD4. The original re-materializes the pool address for the
 *           flags load and again for the store, and returns 0xFF once every
 *           record is occupied. Its two callers, at 0x801EF134 and 0x801EF24C
 *           in func_801EF080, mask the result to a byte and index the same pool
 *           with it to fill record bytes 0x1, 0x9 and 0xB; the sweep at
 *           0x801EEC90 walks the same 36 records and treats that bit 0 as the
 *           in-use flag, and the reset at 0x801EED40 clears bytes 0 through 2
 *           of every record. Takes no arguments and reads or writes no state
 *           beyond the flags byte. Leaf with no frame.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic05803_801EFE38(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x24; index++) {
        offset = index * 0x78;
        flags = D_801EFEF4[offset];
        if (!(flags & 1)) {
            D_801EFEF4[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
