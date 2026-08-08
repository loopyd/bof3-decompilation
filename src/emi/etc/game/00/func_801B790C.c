#include "internal.h"

/*
 * @behavior Dispatches the handler table (0x801CD310) indexed by the
 * work-area byte (work+0x04) through a framed jalr call.
 * @source 0x801B790C
 */
void NO_SIBLING_CALLS func_801B790C(void)
{
    D_801CD310[g_game_work->field_04]();
}
