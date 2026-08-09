#include "bof3/battle/battle15_internal.h"

/* @source 0x800B0CE4
 * @behavior copies five local panel-task handlers, then dispatches by the
 * panel-task state byte at offset 0x03.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchPanelTaskTable6a14(void)
{
    BattlePanelTaskDispatchTable handlers;

    handlers = D_80096A14;
    handlers.handlers[D_80148648->unk_00[3]]();
}
