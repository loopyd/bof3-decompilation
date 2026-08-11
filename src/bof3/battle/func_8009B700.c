#include "bof3/battle/battle15_internal.h"

/* @source 0x8009B700
 * @behavior Applies the indexed signed coordinate offsets while the gate is clear.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_8009B700(void)
{
    u32 index;
    s16 *x;
    s16 *y;

    if (D_80149332 == 0) {
        x = &D_8014930A;
        y = &D_8014930E;
        D_8014932E = 0x80;
        index = D_801462EC * 2;
        *x -= D_800B44C0[index];
        *y -= D_800B44C0[index + 1];
        D_801462E4 += 2;
    }
}
