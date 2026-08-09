#include "bof3/battle/battle03_internal.h"

/* @source 0x801E4104
 * @behavior Increments scratchpad byte +0x03 when enemyReadyOrHelper2 returns nonzero.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void advanceByte3WhenReady2(void) {
    if (enemyReadyOrHelper2() != 0) {
        SPAD_PTR_SLOT(u8, 0x44)[3]++;
    }
}
