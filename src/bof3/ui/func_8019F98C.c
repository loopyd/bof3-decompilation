#include "bof3/ui/game00_internal.h"

/*
 * @behavior Dispatches the handler table (0x801C81DC) indexed by the
 * work-area byte (work+0x01) through a framed jalr call.
 * @source 0x8019F98C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS func_8019F98C(void)
{
    D_801C81DC[g_game_work->unk_01]();
}
