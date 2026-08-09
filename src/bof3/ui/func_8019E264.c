#include "bof3/ui/game00_internal.h"

/*
 * @behavior Dispatches the handler table (0x801C8198) indexed by the
 * work-area byte (work+0x02) through a framed jalr call.
 * @source 0x8019E264
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS func_8019E264(void)
{
    D_801C8198[g_game_work->flags_02]();
}
