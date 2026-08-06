#include "internal.h"

/* @source 0x800A83F8
 * @behavior dispatches one of two local battle-selection handlers by the
 * byte at offset 0x01 in the scratchpad battle work area.
 */
void NO_SIBLING_CALLS dispatchWorkByte1Pair(void)
{
    BattleSelectionHandler handlers[2];

    barrier();
    handlers[0] = initRecordStateAdvanceWork;
    handlers[1] = func_800A84FC;
    handlers[SPAD_PTR_SLOT(u8, 0x44)[1]]();
}
