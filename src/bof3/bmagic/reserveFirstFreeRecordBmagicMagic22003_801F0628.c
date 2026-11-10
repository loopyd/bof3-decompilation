#include "base/types.h"

extern u8 D_801F0710[]; /* @source 0x801F0710 @kind data */

/* @source 0x801F0628
 * @behavior Reserves the first of 64 records in the pool at 0x801F0710 whose
 *           flags byte has bit 0 clear, sets that bit and returns its index,
 *           or returns 0xFF when every record is occupied. Records have stride
 *           0x78. The caller at 0x801EF2F4 masks the result to a byte and tests
 *           it against 0xFF before indexing this pool. Takes no arguments and
 *           touches only the selected flags byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic22003_801F0628(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x40; index++) {
        offset = index * 0x78;
        flags = D_801F0710[offset];
        if (!(flags & 1)) {
            D_801F0710[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
