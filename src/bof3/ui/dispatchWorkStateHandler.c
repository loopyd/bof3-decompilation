#include "bof3/ui/game00_internal.h"

/* @source 0x801ACEBC @behavior dispatches the selected local state handler
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchWorkStateHandler(void)
{
    GameEntry0StateHandler handlers[] = {
        func_801ACF2C,
        func_801AD0EC,
        func_801AD184,
        func_801AD218,
        func_801AD2CC
    };

    handlers[g_game_work->field_04]();
}
