#include "bof3/ui/game00_internal.h"

extern u32 func_8014D8D4(u8 arg0);

/*
 * @source 0x801B73F4
 * @behavior Selects sequence 0x38 when the scratch work route index is 3 and sequence
 * 0x39 otherwise through func_8014D8D4. Selecting the current sequence does
 * not restart it. Writes factor-table index 3 at D_80146250 + 0x11C and
 * advances the scratch handler byte at 0x03 to 1. The next D_801CD2F4 entry
 * is func_801B744C, which reads the D_80181BD4 factor table.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectWorkSequenceAndSetFactorIndex3(void)
{
    struct GameWorkArea* work;

    work = SPAD_PTR_SLOT(struct GameWorkArea, 0x44u);
    if (work->route_index_08 == 3) {
        func_8014D8D4(0x38u);
    } else {
        func_8014D8D4(0x39u);
    }
    D_80146250[0x11C] = 3;
    g_game_work->pad_03 = 1;
}
