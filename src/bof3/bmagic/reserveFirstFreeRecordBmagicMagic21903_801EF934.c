#include "bof3/bof3.h"

extern u8 magic21903RecordFlags[]; /* @source 0x801EFA14 @kind data */

/* @source 0x801EF934
 * @behavior Reserves the first of the 32 records in the pool at 0x801EFA14
 *           whose flags byte has bit 0 clear, sets that bit and returns the
 *           index, or returns 0xFF once every record is occupied. Records have
 *           stride 0x78, so the pool covers 0xF00 bytes from 0x801EFA14 to the
 *           end of the payload at 0x801F0914, 0x2C bytes into the data blob
 *           D_801EF9E8. The pool address is re-materialized for the flags load
 *           and again for the store. The sweep at 0x801EEC88 in func_801EEC14
 *           tests that same bit 0 as the in-use flag and, for an occupied
 *           record, reads its word at offset 0x74 into the dispatch word. The
 *           only in-image call is the jal at 0x801EEE64 in func_801EEE50, which
 *           masks the result to a byte, leaves its loop on 0xFF, and otherwise
 *           clears record byte 0x1, stores the loop index into byte 0xB,
 *           publishes the work pointer into word 0x74 and seeds byte 0x9 from
 *           blob byte D_801EF9E8[index] plus one plus one random bit. Takes no
 *           arguments and reads and writes no state beyond its own flags byte.
 *           The statement shape is that of the sibling lifts at
 *           emi/bmagic/magic081/03 0x801EFA20, emi/bmagic/magic013/03
 *           0x801F0358 and emi/bmagic/magic213/03 0x801EF7EC, which differ only
 *           in pool base and record count.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic21903_801EF934(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x20; index++) {
        offset = index * 0x78;
        flags = magic21903RecordFlags[offset];
        if (!(flags & 1)) {
            magic21903RecordFlags[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
