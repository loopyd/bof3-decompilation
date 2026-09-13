#include "bof3/battle/battle15_internal.h"

/* @source 0x800AD69C
 * @behavior copies the three handlers at 0x800969A0 and dispatches the entry
 * selected by battle work byte 0x01.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkTable69a0(void)
{
    BattleSelectionDispatchTable table;

    u8* work;

    table = D_800969A0;
    work = g_battle_work;
    D_801459F0 = 0x800F0800;
    table.handlers[work[1]]();
    D_801459F0 = 0x800D3800;
}
