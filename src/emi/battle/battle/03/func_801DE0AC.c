#include "internal.h"

/* @source 0x801DE0AC */
/* @behavior Multiplies signed inputs, divides by 100, clamps to 0 through 999, and returns s16. */
s16 func_801DE0AC(s32 arg0, s32 arg1) {
    s32 value;

    value = (arg0 * arg1) / 100;
    if (value >= 1000) {
        value = 999;
    }
    if (value < 0) {
        value = 0;
    }
    return value;
}
