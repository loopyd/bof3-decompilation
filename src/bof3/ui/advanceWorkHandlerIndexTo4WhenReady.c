#include "bof3/ui/game00_internal.h"

/*
 * @behavior Runs the shared work-area service func_801C4F88 and, when it
 * reports success, publishes handler index 4 into the work-area byte at
 * offset 0x04, the handler index of the D_801CD330 work-state dispatch table.
 * Entry 3 of that table (0x801CD330 + 12): entry 2 publishes 3, this one
 * publishes 4.
 * @source 0x801B7DFC
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkHandlerIndexTo4WhenReady(void)
{
    if (func_801C4F88() != 0u) {
        g_game_work->field_04 = 4;
    }
}
