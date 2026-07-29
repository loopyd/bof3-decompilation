#include "internal.h"

void func_800A5120(void) {
    u8 *state;

    if (!(D_801485B8 & 1)) {
        state = &D_801462E5;
        D_801462E3 = 2;
        D_801462E4 = 0;
        *state &= 0x7F;
    }
}
