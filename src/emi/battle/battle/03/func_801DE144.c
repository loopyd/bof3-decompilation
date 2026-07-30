#include "internal.h"

/* @source 0x801DE144 */
/* @behavior Multiplies signed inputs, divides the product by 100, clamps the result to [0, 100], and returns s16. */
s16 func_801DE144(s32 arg0, s32 arg1) {
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
