#include "bof3/ui/game00_internal.h"

/*
 * @behavior Runs the shared tint-fade ramp func_801C27E0 with the 4-unit delta
 * and, when that step reports the fade complete (non-zero byte result), clears
 * bit 0x04 of the record byte at D_80146250 + 0x118 and publishes handler index
 * 1 into the work-area handler index byte at 0x03; nothing happens while the
 * fade is not complete.
 * @source 0x801B707C
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearStatusBit2AndAdvanceHandlerIndexTo1WhenTintFadeCompletes(void)
{
    if (func_801C27E0(4) != 0u) {
        D_80146250[0x118] &= 0xFB;
        g_game_work->pad_03 = 1;
    }
}
