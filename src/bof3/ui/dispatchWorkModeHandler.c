#include "bof3/ui/game00_internal.h"

/*
 * @behavior Dispatches the mode handler table (0x801C88FC) indexed by the
 * work-area mode byte (work+0x04) through a tail-style jalr call.
 * @source 0x801AC9D0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchWorkModeHandler(void)
{
    D_801C88FC[g_game_work->field_04]();
}
