#include "bof3/battle/battle15_internal.h"

/* @source 0x800A83F8
 * @behavior dispatches one of two local battle-selection handlers by the
 * byte at offset 0x01 in the scratchpad battle work area.
 * @status exact
 * @match 100.00
 * @residual none
 */
void NO_SIBLING_CALLS dispatchWorkByte1Pair(void)
{
    BattleSelectionHandler handlers[2] = { initRecordStateAdvanceWork,
                                           raiseFlag4WhenPendingBitsClear };

    handlers[SPAD_PTR_SLOT(u8, 0x44)[1]]();
}
