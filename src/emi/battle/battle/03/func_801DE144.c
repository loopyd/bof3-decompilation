#include "internal.h"

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
