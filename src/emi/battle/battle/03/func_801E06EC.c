#include "internal.h"

/* @source 0x801E06EC
 * @behavior Increments scratchpad work byte +3 and calls func_801DEFE4 unless local-work flags +0x80 contain bit 2.
 */
void func_801E06EC(void) {
    Battle03LocalWork* work;

    work = SPAD_PTR_SLOT(Battle03LocalWork, 0x44);
    work->unk_03++;
    if (!(D_80146250->unk_80 & 4)) {
        func_801DEFE4();
    }
}
