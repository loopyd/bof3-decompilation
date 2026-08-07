#include "internal.h"

/*
 * @behavior Dispatches the handler table (0x801C81B8) indexed by the
 * work-area byte (work+0x02) through a framed jalr call.
 * @source 0x8019E918
 */
void NO_SIBLING_CALLS func_8019E918(void)
{
    D_801C81B8[g_game_work->flags_02]();
}
