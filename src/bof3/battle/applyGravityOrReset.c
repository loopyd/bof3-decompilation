#include "bof3/battle/battle03_internal.h"

/* @source 0x801E48D4
 * @behavior Adds local-work fields +0x10 and +0x1C to field +0x44, then calls func_801E54EC when field +0x44 is negative.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void applyGravityOrReset(void)
{
    Battle03LocalWork *work;

    work = (Battle03LocalWork *)SPAD_PTR_SLOT(u8, 0x44);
    work->unk_44 += work->unk_10;
    work->unk_10 += work->unk_1c;
    if (work->unk_44 < 0) {
        func_801E54EC();
    }
}
