#include "internal.h"

void func_801E47E0(void) {
    Battle03LocalWork* work;

    work = (Battle03LocalWork*)D_1F800044;
    if ((s32)work->unk_40 <= 0x13FFF) {
        work->unk_40 += work->unk_0c;
        work->unk_44 += work->unk_0c;
        work->unk_0c += work->unk_18;
    } else {
        work->unk_03 = 2;
    }
}
