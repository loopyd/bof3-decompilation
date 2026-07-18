#include "internal.h"

/* D_801455C8: 480-byte region, accessed with stride-8 byte offsets */
extern volatile u8 D_801455C8[];

/* @source 0x801F02E4
 * @behavior counts non-zero bytes in D_801455C8 region with stride-8, returns masked count
 */
u8 func_801F02E4(void) {
    s32 count = 0;
    s32 v1 = 0;

    do {
        if (D_801455C8[v1] != 0) {
            count++;
        }
        v1 += 8;
    } while (v1 < 0x1E0);

    return (u8)(count & 0xFF);
}
