#include "internal.h"

/*
 * @behavior Dispatches the handler table (0x801C817C) indexed by the
 * work-area byte (work+0x02) through a framed jalr call.
 * @source 0x8019DE10
 */
void NO_SIBLING_CALLS func_8019DE10(void)
{
    D_801C817C[g_game_work->flags_02]();
}
