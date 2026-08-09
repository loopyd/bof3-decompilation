#include "bof3/battle/battle15_internal.h"

/* @source 0x800ADFBC
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
// @behavior Dispatches an action-table handler when scratchpad state 5 is nonzero.
void func_800ADFBC(void)
{
    if (D_801462E8 & 0x800) {
        D_800B65FC[g_battle_work[5]].handler();
    }
}
