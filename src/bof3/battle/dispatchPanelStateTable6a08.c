#include "bof3/battle/battle15_internal.h"

/* @source 0x800B09CC
 * @behavior copies the locally owned three-word dispatch table at 0x80096A08,
 * then calls the void(void) entry selected by panel-task state byte +0x03.
 * Original table words are 0x800B0A2C, 0x800B0A54, and 0x800B0AB0.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchPanelStateTable6a08(void)
{
    BattleSelectionDispatchTable handlers;

    handlers = D_80096A08;
    handlers.handlers[D_80148648->state]();
}
