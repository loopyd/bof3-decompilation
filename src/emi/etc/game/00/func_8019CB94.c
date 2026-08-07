#include "internal.h"

/*
 * @behavior Dispatches the handler table (0x801C8154) indexed by the
 * work-area byte (work+0x01) through a framed jalr call.
 * @source 0x8019CB94
 */
void NO_SIBLING_CALLS func_8019CB94(void)
{
    D_801C8154[g_game_work->unk_01]();
}
