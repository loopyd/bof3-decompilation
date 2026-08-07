#include "internal.h"

/*
 * @behavior Dispatches the handler table (0x801CD37C) indexed by the
 * work-area byte (work+0x03) through a framed jalr call.
 * @source 0x801B91CC
 */
void NO_SIBLING_CALLS func_801B91CC(void)
{
    D_801CD37C[g_game_work->pad_03[0]]();
}
