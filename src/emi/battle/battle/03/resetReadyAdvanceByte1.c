#include "internal.h"

/* @source 0x801DF3B8
 * @behavior Calls func_801DEFE4 then localReadyOrHelper1 and increments scratchpad work byte +1.
 */
void resetReadyAdvanceByte1(void) {
    func_801DEFE4();
    localReadyOrHelper1();
    SPAD_PTR_SLOT(u8, 0x44)[1]++;
}
