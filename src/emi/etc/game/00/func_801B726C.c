#include "internal.h"

/*
 * @behavior Dispatches the handler table (0x801CD2EC) indexed by the
 * work-area byte (work+0x03) through a framed jalr call.
 * @source 0x801B726C
 */
void NO_SIBLING_CALLS func_801B726C(void)
{
    D_801CD2EC[g_game_work->pad_03]();
}
