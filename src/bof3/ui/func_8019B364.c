#include "bof3/ui/game00_internal.h"

/*
 * @behavior Dispatches the handler table (0x801C812C) indexed by the
 * work-area byte (work+0x01) through a framed jalr call.
 * @source 0x8019B364
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS func_8019B364(void)
{
    D_801C812C[g_game_work->unk_01]();
}
