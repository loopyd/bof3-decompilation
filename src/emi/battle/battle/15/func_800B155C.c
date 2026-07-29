#include "internal.h"

/* @source 0x800B155C
 * @behavior copies the locally owned three-word dispatch table at 0x80096A40,
 * then calls the void(void) entry selected by panel-task state byte +0x03.
 * Original table words are 0x800B15BC, 0x800B1664, and 0x800B16D0.
 */
void func_800B155C(void)
{
    BattleSelectionDispatchTable handlers;

    handlers = D_80096A40;
    handlers.handlers[D_80148648->state]();
}
