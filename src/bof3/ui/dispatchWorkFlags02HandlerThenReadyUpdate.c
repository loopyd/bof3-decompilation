#include "bof3/ui/game00_internal.h"

/*
 * @source 0x801B371C
 * @behavior Dispatches the two-entry handler table (0x801CD1B0) indexed by the
 * work-area flags byte (work+0x02) through a framed jalr call, then runs the
 * shared ready helper func_8014D978.
 * @status exact
 * @match 100.00
 * @residual none
 */
void NO_SIBLING_CALLS dispatchWorkFlags02HandlerThenReadyUpdate(void)
{
    D_801CD1B0[g_game_work->flags_02]();
    func_8014D978();
}
