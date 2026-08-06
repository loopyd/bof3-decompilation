#include "internal.h"

/* @source 0x801E4DD8
 * @behavior Calls battle03_enemy_ready_or_helper2 and increments scratchpad work byte +0x02.
 */
void battle03_check_ready2_advance_byte2(void) {
    battle03_enemy_ready_or_helper2();
    D_1F800044->unk_02++;
}
