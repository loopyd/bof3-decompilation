#include "bof3/ui/game00_internal.h"

extern u32 func_8014D8D4(u8 arg0);

/*
 * @source 0x801C4D1C
 * @behavior Selects sequence 0x3C when the scratch work route index is 7 and sequence
 * 0x3D otherwise through func_8014D8D4. Selecting the current sequence does
 * not restart it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void selectWorkSequenceForRoute7(void)
{
    struct GameWorkArea* work;

    work = SPAD_PTR_SLOT(struct GameWorkArea, 0x44u);
    if (work->route_index_08 == 7) {
        func_8014D8D4(0x3Cu);
    } else {
        func_8014D8D4(0x3Du);
    }
}
