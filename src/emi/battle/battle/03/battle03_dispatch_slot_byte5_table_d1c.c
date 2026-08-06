#include "internal.h"

// @source 0x801E5B8C
// @behavior Copies the local handler table then dispatches by battle work byte 5.
void battle03_dispatch_slot_byte5_table_d1c(void)
{
    Battle03SeventyDispatchTable table;

    table = D_801D0D1C;
    table.handlers[D_801EC2E0->unk_05]();
}
