#include "internal.h"

/* @source 0x801ACEBC @behavior dispatches the selected local state handler */
void func_801ACEBC(void)
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
