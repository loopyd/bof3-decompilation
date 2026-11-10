#include "base/types.h"

extern u8 D_801F03BC[]; /* @source 0x801F03BC @kind data */

/* @source 0x801F02D4
 * @behavior Reserves the first of 32 records in the pool at 0x801F03BC whose
 *           flags byte has bit 0 clear, sets that bit and returns its index,
 *           or returns 0xFF when every record is occupied. Records have stride
 *           0x78. The caller at 0x801EEECC masks the result to a byte and tests
 *           it against 0xFF before indexing this pool. The sweep at 0x801EEC14
 *           tests bit 0 as the in-use flag. Takes no arguments and touches
 *           only the selected flags byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic22503_801F02D4(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x20; index++) {
        offset = index * 0x78;
        flags = D_801F03BC[offset];
        if (!(flags & 1)) {
            D_801F03BC[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
