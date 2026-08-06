#include "internal.h"

// @source 0x801E5B8C
// @behavior Copies the local handler table then dispatches by battle work byte 5.
void dispatchSlotByte5TableD1c(void)
{
    Battle03SeventyDispatchTable table;

    table = D_801D0D1C;
    table.handlers[D_801EC2E0->unk_05]();
}
