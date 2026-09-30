#include "bof3/battle/battle03_internal.h"

/* @source 0x801E46EC
 * @behavior Increments scratchpad work byte +0x03 when the enemy readiness
 * helper enemyReadyOrHelper2 reports success, then runs the enemy script byte
 * helper func_801E2314 with 8 when the current enemy work word +0x100 has bit
 * 0x02 set, otherwise with 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E46EC(void) {
  if (enemyReadyOrHelper2() != 0) {
    D_1F800044->unk_03++;
    if ((BATTLE_ENEMY_WORD_100(BATTLE_CURRENT_ENEMY_PTR) & 2u) != 0u) {
      func_801E2314(8);
    } else {
      func_801E2314(0);
    }
  }
}
