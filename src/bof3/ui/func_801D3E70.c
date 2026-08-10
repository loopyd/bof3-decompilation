#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D3E70
 * @behavior advances a local counter unless state is 2, then selects mode 2
 * when the counter reaches the main-state limit and sets handler index 1.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_801D3E70(void)
{
    u8 value;

    if (D_80143BB0 == 2) {
        return;
    }

    value = D_801D428A + 1;
    D_801D428A = value;
    if (value >= D_80146254) {
        modeIndex = 2;
    }
    D_801D4286 = 1;
}
