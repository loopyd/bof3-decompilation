#include "bof3/ui/game00_internal.h"

/* @source 0x801B8578
 * @behavior Runs the shared tint-fade step func_801C27E0 with the +8 ramp
 * delta and, when that step reports the fade complete (non-zero byte), advances
 * the work-area handler index byte at 0x03 by one - the index byte of the
 * D_801CD140 / D_801CD154 dispatch tables. Nothing else is touched, so this is
 * the 0x03 sibling of advanceWorkHandlerIndexWhenTintFadeCompletes (0x04).
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceHandlerIndexWhenTintFadeCompletes(void)
{
    if (func_801C27E0(8) != 0u) {
        g_game_work->pad_03++;
    }
}
