#include "internal.h"

void func_801EA728(void) {
    u8* panel = (u8*)D_80148648;
    s16 state = FIELD_REF(s16, panel, 6);

    if (state == 0x12) {
        FIELD_REF(u8, panel, 3)++;
    } else {
        FIELD_REF(s16, panel, 6) = state + 8;
    }
}
