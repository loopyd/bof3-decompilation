#include "internal.h"

/* @source 0x801E4104
 * @behavior Increments scratchpad byte +0x03 when battle03_enemy_ready_or_helper2 returns nonzero.
 */
void battle03_advance_byte3_when_ready2(void) {
    if (battle03_enemy_ready_or_helper2() != 0) {
        SPAD_PTR_SLOT(u8, 0x44)[3]++;
    }
}
