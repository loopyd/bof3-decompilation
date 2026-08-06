#include "internal.h"

/* @source 0x801E4104
 * @behavior Increments scratchpad byte +0x03 when enemyReadyOrHelper2 returns nonzero.
 */
void advanceByte3WhenReady2(void) {
    if (enemyReadyOrHelper2() != 0) {
        SPAD_PTR_SLOT(u8, 0x44)[3]++;
    }
}
