#include "bof3/battle/battle15_internal.h"

/* @source 0x800AD9CC
 * @behavior copies the three handlers at 0x800969AC and dispatches the entry
 * selected by battle work byte 0x01.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkTable69ac(void)
{
    BattleSelectionDispatchTable table;

    u8* work;

    table = D_800969AC;
    work = g_battle_work;
    D_801459F0 = 0x800F0800;
    table.handlers[work[1]]();
    D_801459F0 = 0x800D3800;
}
