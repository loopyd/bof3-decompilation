#include "bof3/ui/game00_internal.h"

/*
 * @behavior Runs the shared work-area service func_801C4F88 and, when it
 * reports success, publishes handler index 4 into the work-area byte at
 * offset 0x03, the handler index of the D_801CD140/D_801CD154 dispatch tables.
 * @source 0x801B3400
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHandlerIndexTo4WhenReady(void)
{
    if (func_801C4F88() != 0u) {
        g_game_work->pad_03 = 4;
    }
}
