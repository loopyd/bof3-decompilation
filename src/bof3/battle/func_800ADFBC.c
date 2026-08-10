#include "bof3/battle/battle15_internal.h"

/* @source 0x800ADFBC
 * @behavior Dispatches an action-table handler when flag 0x800 is set and indexes it by battle-work byte 5.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void func_800ADFBC(void)
{
    if (D_801462E8 & 0x800) {
        battleSelectionActionTable[g_battle_work[5]].handler();
    }
}
