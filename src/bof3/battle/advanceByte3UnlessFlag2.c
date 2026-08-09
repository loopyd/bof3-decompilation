#include "bof3/battle/battle03_internal.h"

/* @source 0x801E06EC
 * @behavior Increments scratchpad work byte +3 and calls func_801DEFE4 unless local-work flags +0x80 contain bit 2.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void advanceByte3UnlessFlag2(void) {
    Battle03LocalWork* work;

    work = SPAD_PTR_SLOT(Battle03LocalWork, 0x44);
    work->unk_03++;
    if (!(D_80146250->unk_80 & 4)) {
        func_801DEFE4();
    }
}
