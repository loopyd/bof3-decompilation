#include "internal.h"

/*
 * @behavior Dispatches the handler table (0x801C80F8) indexed by the
 * work-area byte (work+0x01) through a framed jalr call.
 * @source 0x8019ADA4
 */
void NO_SIBLING_CALLS func_8019ADA4(void)
{
    D_801C80F8[g_game_work->unk_01]();
}
