#include "bof3/ui/game00_internal.h"

/* @source 0x801B7C10
 * @behavior runs the shared work-area readiness service func_801C520C and,
 * when it reports success, advances the work-state handler index byte at
 * g_game_work + 0x04 by one; the first of the two work-state advance gates
 * (the second is the sibling at 0x801B7C58, gated by func_801C539C).
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkHandlerIndexWhenReady(void)
{
    if (func_801C520C() != 0u) {
        g_game_work->field_04++;
    }
}
