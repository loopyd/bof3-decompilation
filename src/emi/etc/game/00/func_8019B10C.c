#include "internal.h"

/*
 * @behavior Dispatches the handler table (0x801C8120) indexed by the
 * work-area byte (work+0x01) through a framed jalr call.
 * @source 0x8019B10C
 */
void NO_SIBLING_CALLS func_8019B10C(void)
{
    D_801C8120[g_game_work->unk_01]();
}
