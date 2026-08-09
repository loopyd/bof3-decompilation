#include "bof3/battle/battle03_internal.h"

/* @source 0x801E19DC
 * @behavior Calls localReadyOrHelper2 and increments byte 2 of scratchpad pointer slot 0x44.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void checkReadyAdvanceByte2(void) {
    localReadyOrHelper2();
    SPAD_PTR_SLOT(u8, 0x44)[2]++;
}
