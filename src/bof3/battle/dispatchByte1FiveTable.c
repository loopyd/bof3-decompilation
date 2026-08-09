#include "bof3/battle/battle03_internal.h"

/* @source 0x801E6C84
 * @behavior copies five handlers locally and dispatches one selected by battle work byte 1.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS dispatchByte1FiveTable(void)
{
    Battle03FiveDispatchTable handlers;

    handlers = D_801D0F44;
    if (D_801462E0 != 5) {
        D_801459F0 = 0x800F0800;
        handlers.handlers[SPAD_PTR_SLOT(u8, 0x44)[1]]();
        D_801459F0 = 0x800D3800;
    }
}
