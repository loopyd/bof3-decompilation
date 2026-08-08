#include "internal.h"

/*
 * @behavior Dispatches the handler table (0x801CD330) indexed by the
 * work-area byte (work+0x04) through a framed jalr call.
 * @source 0x801B7D00
 */
void NO_SIBLING_CALLS func_801B7D00(void)
{
    D_801CD330[g_game_work->field_04]();
}
