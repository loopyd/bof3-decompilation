#include "bof3/battle/battle03_internal.h"

/* @source 0x801DE144
 * @behavior Multiplies the signed inputs, divides the signed low-word product
 * by 100, clamps the result to 0 through 100 and returns it as s16.
 * @status exact
 * @match 100.00
 * @residual none
 */
s16 scalePercentClamp100(s32 arg0, s32 arg1) {
    s32 value;

    value = (arg0 * arg1) / 100;
    if (value >= 101) {
        value = 100;
    }
    if (value < 0) {
        value = 0;
    }
    return value;
}
