#include "bof3/ui/game00_internal.h"

/*
 * @behavior Dispatches the handler table (0x801CD154) indexed by the
 * work-area byte (work+0x03) through a framed jalr call.
 * @source 0x801B3540
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS func_801B3540(void)
{
    D_801CD154[g_game_work->pad_03]();
}
