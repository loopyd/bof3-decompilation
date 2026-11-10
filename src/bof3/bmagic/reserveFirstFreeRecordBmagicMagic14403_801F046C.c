#include "bof3/bof3.h"

extern u8 D_801F050C[]; /* @source 0x801F050C @kind data */

/* @source 0x801F046C
 * @behavior Reserves the first of 48 records in the pool at 0x801F050C whose
 *           flags byte has bit 0 clear, sets that bit and returns its index.
 *           Records have stride 0x78. Returns 0xFF when all records are occupied.
 *           The caller at 0x801EEF68 masks the result to a byte, tests it against
 *           0xFF and indexes this pool with the same stride. Takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 reserveFirstFreeRecordBmagicMagic14403_801F046C(void) {
    u8 index;
    u8 flags;
    u32 offset;

    for (index = 0; index < 0x30; index++) {
        offset = index * 0x78;
        flags = D_801F050C[offset];
        if (!(flags & 1)) {
            D_801F050C[offset] = flags | 1;
            return index;
        }
    }
    return 0xFF;
}
