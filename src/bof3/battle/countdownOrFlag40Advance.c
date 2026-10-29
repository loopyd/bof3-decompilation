#include "bof3/battle/battle03_internal.h"

/* @source 0x801E7558
 * @behavior Subtracts 0x2000 from word +0x40 of the work record at D_801EB4E0
 * when that word is nonzero. If it is already zero, sets bit 0x40 of work byte
 * +0x00 and increments byte +0x01 through the scratchpad pointer cell at
 * 0x1F800044.
 * @status exact
 * @match 100.00
 * @residual none
 */
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
