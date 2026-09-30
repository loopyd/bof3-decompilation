#include "bof3/ui/game00_internal.h"

/*
 * @behavior Runs the shared tint step func_801C29A0 with the 4-unit delta and,
 * when that step reports that all three signed tint channels have reached the
 * -0x80 floor (non-zero byte result), clears bit 0x02 of the record byte at
 * D_80146250 + 0x118 and publishes handler index 1 into the work-area handler
 * index byte at 0x03; nothing happens while the floor is not reached.
 * @source 0x801B7024
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearStatusBit1AndAdvanceHandlerIndexTo1WhenTintFadeCompletes(void)
{
    if ((u8)func_801C29A0(4) != 0) {
        D_80146250[0x118] &= 0xFD;
        g_game_work->pad_03 = 1;
    }
}
