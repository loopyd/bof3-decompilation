#include "internal.h"

/* @source 0x801DF3B8
 * @behavior Calls func_801DEFE4 then func_801DEDE4 and increments scratchpad work byte +1.
 */
void func_801DF3B8(void) {
    func_801DEFE4();
    func_801DEDE4();
    SPAD_PTR_SLOT(u8, 0x44)[1]++;
}
