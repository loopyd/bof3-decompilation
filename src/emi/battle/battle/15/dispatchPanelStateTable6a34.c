#include "internal.h"

/* @source 0x800B138C
 * @behavior copies the locally owned three-word dispatch table at 0x80096A34,
 * then calls the void(void) entry selected by panel-task state byte +0x03.
 * Original table words are 0x800B13EC, 0x800B14BC, and 0x800B1504.
 */
void dispatchPanelStateTable6a34(void)
{
    BattleSelectionDispatchTable handlers;

    handlers = D_80096A34;
    handlers.handlers[D_80148648->state]();
}
