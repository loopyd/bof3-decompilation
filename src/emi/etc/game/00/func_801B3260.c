#include "internal.h"

/*
 * @behavior Dispatches the handler table (0x801CD130) indexed by the
 * work-area byte (work+0x02) through a framed jalr call.
 * @source 0x801B3260
 */
void NO_SIBLING_CALLS func_801B3260(void)
{
    D_801CD130[g_game_work->flags_02]();
}
