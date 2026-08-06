#include "internal.h"

/* @source 0x800AD26C
 * @behavior copies the three handlers at 0x80096994 and dispatches the entry
 * selected by battle work byte 0x01.
 */
void battle15_dispatch_work_table_6994(void)
{
    BattleSelectionDispatchTable handlers;
    /* MATCHING_AID: original retains g_battle_work in v1 for the stack-table index. */
    REGISTER_PIN(u8*, work, "v1");

    handlers = D_80096994;
    work = g_battle_work;
    D_801459F0 = 0x800F0800;
    ((BattleSelectionHandler *)&handlers)[work[1]]();
    D_801459F0 = 0x800D3800;
}
