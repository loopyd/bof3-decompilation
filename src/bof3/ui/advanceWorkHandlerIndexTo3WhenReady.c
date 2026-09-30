#include "bof3/ui/game00_internal.h"

/*
 * @behavior Runs the shared work-area service func_801C4EC4 and, when it
 * reports success, publishes handler index 3 into the work-area byte at
 * offset 0x04, the handler index of the D_801CD330 work-state dispatch table.
 * Entry 2 of that table (0x801CD330 + 8): entries 0-1 publish 1-2, this one
 * publishes 3.
 * @source 0x801B7DC0
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkHandlerIndexTo3WhenReady(void)
{
    if (func_801C4EC4() != 0u) {
        g_game_work->field_04 = 3;
    }
}
