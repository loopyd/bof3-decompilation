#include "bof3/ui/game00_internal.h"

/*
 * @source 0x801B7E38
 * @behavior Runs the shared work-area service func_801C4FE0 and, when it
 * reports success, publishes handler index 5 into the work-area byte at
 * offset 0x04, the handler index of the D_801CD330 work-state dispatch table.
 * Entry 4 of that table (0x801CD330 + 16): entry 0 publishes 1, entries 1-3
 * publish 2-4 through the services func_801C4E14, func_801C4EC4 and
 * func_801C4F88, and this entry publishes 5 through func_801C4FE0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkHandlerIndexTo5WhenReady(void)
{
    if (func_801C4FE0() != 0u) {
        g_game_work->field_04 = 5;
    }
}
