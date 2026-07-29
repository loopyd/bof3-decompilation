#include "internal.h"

void func_801E7558(void) {
    Battle03LocalWork* work;

    work = D_801EB4E0;
    if (work->unk_40 != 0) {
        work->unk_40 -= 0x2000;
    } else {
        work->flags_00 |= 0x40;
        SPAD_PTR_SLOT(u8, 0x44)[1]++;
    }
}
