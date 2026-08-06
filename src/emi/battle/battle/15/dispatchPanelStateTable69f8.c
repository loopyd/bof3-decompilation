#include "internal.h"

/* @source 0x800B01F0
 * @behavior copies the three local handlers at 0x800969F8, then dispatches
 * by the panel-task state byte at offset 0x03.
 */
void dispatchPanelStateTable69f8(void)
{
    BattleSelectionDispatchTable handlers;

    handlers = D_800969F8;
    handlers.handlers[D_80148648->state]();
}
