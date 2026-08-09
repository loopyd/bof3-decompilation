#include "bof3/battle/battle03_internal.h"

/* @source 0x801E949C
 * @behavior Calls func_801D8450 with D_801462F4 when D_80144955 is zero.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void callD8450WhenIdle(void)
{
    if (D_80144955 == 0) {
        func_801D8450(D_801462F4);
    }
}
