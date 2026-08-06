#include "internal.h"

/* @source 0x801E78F0
 * @behavior Decrements the active work countdown or flags it and advances scratchpad work.
 */
void countdownOrFlag40Advance2(void) {
    Battle03LocalWork* work;

    work = D_801EB4E0;
    if (work->unk_40 != 0) {
        work->unk_40 -= 0x2000;
    } else {
        work->flags_00 |= 0x40;
        SPAD_PTR_SLOT(u8, 0x44)[1]++;
    }
}
