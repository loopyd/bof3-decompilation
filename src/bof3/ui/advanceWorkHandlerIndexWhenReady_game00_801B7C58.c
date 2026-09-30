#include "bof3/ui/game00_internal.h"

/* @source 0x801B7C58
 * @behavior runs the shared work-area readiness service func_801C539C and,
 * when it reports success, advances the work-state handler index byte at
 * g_game_work + 0x04 by one; the second work-state advance gate after the
 * sibling at 0x801B7C10, which is gated by func_801C520C instead.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkHandlerIndexWhenReady_game00_801B7C58(void)
{
    if (func_801C539C() != 0u) {
        g_game_work->field_04++;
    }
}
