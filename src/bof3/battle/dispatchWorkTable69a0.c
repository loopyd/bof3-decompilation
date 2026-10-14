#include "bof3/battle/battle15_dispatch_internal.h"

/* @source 0x800AD69C
 * @behavior copies the three handlers at 0x800969A0 and dispatches the entry
 * selected by battle work byte 0x01.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkTable69a0(void)
{
    BATTLE_DISPATCH_WORK_TABLE(D_800969A0);
}
