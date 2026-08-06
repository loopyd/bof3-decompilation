#include "internal.h"

/* @source 0x801E4DD8
 * @behavior Calls enemyReadyOrHelper2 and increments scratchpad work byte +0x02.
 */
void checkReady2AdvanceByte2(void) {
    enemyReadyOrHelper2();
    D_1F800044->unk_02++;
}
