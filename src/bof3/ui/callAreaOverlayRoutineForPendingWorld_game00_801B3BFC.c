#include "bof3/ui/game00_internal.h"

/*
 * @behavior Overlay trampoline selected by the pending world id D_80143F00:
 * when that u16 reads 0x68 it calls the concurrently resident per-area overlay
 * routine at 0x801F45BC, otherwise the one at 0x801F4B00.
 * @source 0x801B3BFC
 * @status exact
 * @match 100.00
 * @residual none
 */
void callAreaOverlayRoutineForPendingWorld_game00_801B3BFC(void)
{
    if (D_80143F00 == 0x68) {
        func_801F45BC();
    } else {
        func_801F4B00();
    }
}
