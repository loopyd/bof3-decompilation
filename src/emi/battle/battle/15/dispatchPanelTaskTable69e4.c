#include "internal.h"

/* @source 0x800B0180
 * @behavior copies the five local handlers at 0x800969E4, then dispatches
 * by the panel-task state byte at offset 0x02.
 */
void dispatchPanelTaskTable69e4(void)
{
    BattlePanelTaskDispatchTable handlers;

    handlers = D_800969E4;
    handlers.handlers[D_80148648->unk_00[2]]();
}
