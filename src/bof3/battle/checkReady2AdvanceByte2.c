#include "bof3/battle/battle03_internal.h"

/* @source 0x801E4DD8
 * @behavior Calls enemyReadyOrHelper2 and increments scratchpad work byte +0x02.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void checkReady2AdvanceByte2(void) {
    enemyReadyOrHelper2();
    D_1F800044->unk_02++;
}
