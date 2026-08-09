#include "bof3/battle/battle03_internal.h"

/* @source 0x801DE0F8
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
/* @behavior Multiplies signed inputs, divides by 100, clamps to 0 through 9999, and returns s16. */
s16 scalePercentClamp9999(s32 arg0, s32 arg1) {
    s32 value;

    value = (arg0 * arg1) / 100;
    if (value >= 10000) {
        value = 9999;
    }
    if (value < 0) {
        value = 0;
    }
    return value;
}
