#include "internal.h"

/*
 * @behavior Dispatches the handler table (0x801CD140) indexed by the
 * work-area byte (work+0x03) through a framed jalr call.
 * @source 0x801B32D0
 */
void NO_SIBLING_CALLS func_801B32D0(void)
{
    D_801CD140[g_game_work->pad_03]();
}
