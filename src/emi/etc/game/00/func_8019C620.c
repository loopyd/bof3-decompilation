#include "internal.h"

/*
 * @behavior Dispatches the handler table (0x801C8144) indexed by the
 * work-area byte (work+0x01) through a framed jalr call.
 * @source 0x8019C620
 */
void NO_SIBLING_CALLS func_8019C620(void)
{
    D_801C8144[g_game_work->unk_01]();
}
