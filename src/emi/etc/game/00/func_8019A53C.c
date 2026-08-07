#include "internal.h"

/*
 * @behavior Dispatches the handler table (0x801C80A4) indexed by the
 * work-area byte (work+0x06) through a framed jalr call.
 * @source 0x8019A53C
 */
void NO_SIBLING_CALLS func_8019A53C(void)
{
    D_801C80A4[g_game_work->unk_06]();
}
