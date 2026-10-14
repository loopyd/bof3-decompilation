#include "bof3/battle/battle15_dispatch_internal.h"

/* @source 0x800AD26C
 * @behavior copies the three handlers at 0x80096994 and dispatches the entry
 * selected by battle work byte 0x01.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkTable6994(void)
{
    BATTLE_DISPATCH_WORK_TABLE(D_80096994);
}
