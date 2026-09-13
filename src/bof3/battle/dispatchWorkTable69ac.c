#include "bof3/battle/battle15_internal.h"

/* @source 0x800AD9CC
 * @behavior copies the three handlers at 0x800969AC and dispatches the entry
 * selected by battle work byte 0x01.
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
void dispatchWorkTable69ac(void)
{
    BattleSelectionDispatchTable handlers;

    u8* work;

    handlers = D_800969AC;
    work = g_battle_work;
    D_801459F0 = 0x800F0800;
    ((BattleSelectionHandler *)&handlers)[work[1]]();
    D_801459F0 = 0x800D3800;
}
