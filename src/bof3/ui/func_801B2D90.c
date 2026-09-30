#include "bof3/ui/game00_internal.h"

/*
 * @behavior Dispatches the handler table (0x801CD120) indexed by the
 * work-area flags byte (work+0x02) through a framed jalr call, then runs the
 * shared ready helper func_8014D978.
 * @source 0x801B2D90
 * @status exact
 * @match 100.00
 * @residual none
 */
void NO_SIBLING_CALLS func_801B2D90(void)
{
    D_801CD120[g_game_work->flags_02]();
    func_8014D978();
}
