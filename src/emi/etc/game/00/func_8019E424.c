#include "internal.h"

/*
 * @behavior Dispatches the handler table (0x801C81A0) indexed by the
 * work-area byte (work+0x02) through a framed jalr call.
 * @source 0x8019E424
 */
void NO_SIBLING_CALLS func_8019E424(void)
{
    D_801C81A0[g_game_work->flags_02]();
}
