#include "bof3/battle/battle03_internal.h"

/* @source 0x801DF3B8
 * @behavior Calls forwardSelectedPresentationOffset then localReadyOrHelper1 and increments scratchpad work byte +1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetReadyAdvanceByte1(void) {
    forwardSelectedPresentationOffset();
    localReadyOrHelper1();
    SPAD_PTR_SLOT(u8, 0x44)[1]++;
}
