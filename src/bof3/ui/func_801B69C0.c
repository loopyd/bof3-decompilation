#include "bof3/ui/game00_internal.h"

/*
 * @behavior Dispatches the handler table (0x801CD2B0) indexed by the
 * work-area byte (work+0x02) through a framed jalr call.
 * @source 0x801B69C0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS func_801B69C0(void)
{
    D_801CD2B0[g_game_work->flags_02]();
}
