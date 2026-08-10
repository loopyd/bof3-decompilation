#include "bof3/battle/battle03_internal.h"

/* @source 0x801E5B8C
 * @behavior Copies the local handler table then dispatches by battle work byte 5.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSlotByte5TableD1c(void)
{
    Battle03SeventyDispatchTable table;

    table = D_801D0D1C;
    table.handlers[D_801EC2E0->unk_05]();
}
