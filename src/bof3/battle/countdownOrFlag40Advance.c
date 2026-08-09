#include "bof3/battle/battle03_internal.h"

/* @source 0x801E7558
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
/* @behavior Decrements the active work timer by 0x2000 when nonzero; otherwise sets flag 0x40 and increments scratchpad state. */
void countdownOrFlag40Advance(void) {
    Battle03LocalWork* work;

    work = D_801EB4E0;
    if (work->unk_40 != 0) {
        work->unk_40 -= 0x2000;
    } else {
        work->flags_00 |= 0x40;
        SPAD_PTR_SLOT(u8, 0x44)[1]++;
    }
}
