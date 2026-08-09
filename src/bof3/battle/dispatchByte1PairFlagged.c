#include "bof3/battle/battle03_internal.h"

/* @source 0x801E6724
 * @behavior sets the temporary battle global, dispatches one of two local
 * handlers selected by work byte 1, then restores the global.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS dispatchByte1PairFlagged(void)
{
    Battle03Handler handlers[2];

    barrier();
    handlers[0] = func_801E679C;
    handlers[1] = func_801E68EC;
    D_801459F0 = 0x800F0800;
    ((Battle03Handler*)handlers)[SPAD_PTR_SLOT(u8, 0x44)[1]]();
    D_801459F0 = 0x800D3800;
}
