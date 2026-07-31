#include "internal.h"

u8 func_801E30B8(s8 arg0) {
    s32 result;

    if (D_801462F3 == 1) {
        result = 0xFF;
    } else {
        result = func_801E29B4((u8)(arg0 + 3));
    }

    return result;
}
