#include "bof3/ui/game00_internal.h"

/* @source 0x801AD35C
 * @status exact
 * @match 100.00
 * @residual none
 */
/* @behavior Dispatches the work mode through a two-entry local handler table. */
void NO_SIBLING_CALLS dispatchWorkModePair(void)
{
    GameEntry0StateHandler handlers[2] = {
        func_801AD3B4,
        func_801AD218
    };
    handlers[g_game_work->field_04]();
}
