#include "bof3/ui/game00_internal.h"

/* @source 0x801B81EC
 * @behavior Runs the shared tint-fade step func_801C27E0 with the +8 ramp
 * delta and, when that step reports the fade complete (non-zero byte), advances
 * the work-area handler index byte at 0x04 by one - the index byte of the
 * D_801CD310 / D_801CD330 dispatch tables. Nothing else is touched.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkHandlerIndexWhenTintFadeCompletes(void)
{
    if (func_801C27E0(8) != 0u) {
        g_game_work->field_04++;
    }
}
