#include "bof3/ui/game00_internal.h"

/*
 * @behavior Overlay trampoline selected by the pending world id D_80143F00:
 * when that u16 reads 0x68 it calls the concurrently resident per-area overlay
 * routine at 0x801F53D4, otherwise the one at 0x801F5918. Both resident
 * routines are work-state-byte-1 dispatchers: each indexes its own overlay
 * handler table with the work-area byte at +0x01 of the scratchpad work record
 * at 0x1F800044.
 * @source 0x8019A330
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchAreaOverlayStateByte1ForPendingWorld_game00_8019A330(void)
{
    if (D_80143F00 == 0x68) {
        func_801F53D4();
    } else {
        func_801F5918();
    }
}
