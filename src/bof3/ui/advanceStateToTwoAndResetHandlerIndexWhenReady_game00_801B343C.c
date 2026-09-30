#include "bof3/ui/game00_internal.h"

/*
 * @behavior Runs the shared work-area service func_801C4FE0 and, when it
 * reports success, publishes state 2 into the work-area byte at offset 0x02,
 * the index of the D_801CD130 dispatch table, and clears the work-area byte at
 * offset 0x03, the handler index of the D_801CD140/D_801CD154 dispatch tables.
 * @source 0x801B343C
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceStateToTwoAndResetHandlerIndexWhenReady_game00_801B343C(void)
{
    if (func_801C4FE0() != 0u) {
        g_game_work->flags_02 = 2;
        g_game_work->pad_03 = 0;
    }
}
