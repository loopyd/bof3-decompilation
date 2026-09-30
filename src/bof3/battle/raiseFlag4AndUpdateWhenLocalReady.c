#include "bof3/battle/battle03_internal.h"

/* @source 0x801DFB64
 * @behavior When the second local readiness helper (localReadyOrHelper2)
 * reports ready, re-allocates one free queued slot with mode bytes 0/2 through
 * func_801E590C while the battle global halfword BATTLE_GLOBAL_HALF_62E8
 * already has bit 0x80 set, forwards the battle byte D_80146384 to
 * func_800A9820, raises bit 0x4 of BATTLE_GLOBAL_HALF_62E8 and dispatches the
 * local work reset/update func_801E1DD4; otherwise it does nothing. Handler
 * index 1 of the local byte-2 handler table D_801EB1B4 dispatched by
 * dispatchLocalByte2Table.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlag4AndUpdateWhenLocalReady(void) {
    u16* flags;

    if (localReadyOrHelper2() != 0) {
        flags = (u16*)&BATTLE_GLOBAL_HALF_62E8;
        if ((*flags & 0x80u) != 0u) {
            func_801E590C(0u, 2u);
        }
        func_800A9820(D_80146384);
        *flags |= 0x4u;
        func_801E1DD4();
    }
}
