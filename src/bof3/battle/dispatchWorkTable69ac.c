#include "bof3/battle/battle15_dispatch_internal.h"

/* @source 0x800AD9CC
 * @behavior copies the three handlers at 0x800969AC and dispatches the entry
 * selected by battle work byte 0x01.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkTable69ac(void)
{
    BATTLE_DISPATCH_WORK_TABLE(D_800969AC);
}
