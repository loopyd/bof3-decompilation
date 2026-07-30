#include "internal.h"

/* @source 0x801E6C84
 * @behavior copies five handlers locally and dispatches one selected by battle work byte 1.
 */
void NO_SIBLING_CALLS func_801E6C84(void)
{
    Battle03FiveDispatchTable handlers;

    handlers = D_801D0F44;
    if (D_801462E0 != 5) {
        D_801459F0 = 0x800F0800;
        handlers.handlers[SPAD_PTR_SLOT(u8, 0x44)[1]]();
        D_801459F0 = 0x800D3800;
    }
}
