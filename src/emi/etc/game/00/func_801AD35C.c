#include "internal.h"

/* @source 0x801AD35C */
/* @behavior Dispatches the work mode through a two-entry local handler table. */
void NO_SIBLING_CALLS func_801AD35C(void)
{
    GameEntry0StateHandler handlers[2] = {
        func_801AD3B4,
        func_801AD218
    };
    handlers[g_game_work->pad_03[1]]();
}
