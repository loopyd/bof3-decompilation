#include "internal.h"

/* @source 0x801E19DC
 * @behavior Calls func_801DEE4C and increments byte 2 of scratchpad pointer slot 0x44.
 */
void func_801E19DC(void) {
    func_801DEE4C();
    SPAD_PTR_SLOT(u8, 0x44)[2]++;
}
