#include "internal.h"

/* @source 0x801DF3B8
 * @behavior Calls func_801DEFE4 then battle03_local_ready_or_helper1 and increments scratchpad work byte +1.
 */
void battle03_reset_ready_advance_byte1(void) {
    func_801DEFE4();
    battle03_local_ready_or_helper1();
    SPAD_PTR_SLOT(u8, 0x44)[1]++;
}
