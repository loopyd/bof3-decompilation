#include "bof3/battle/battle03_internal.h"

/* @source 0x801E5288
 * @behavior Runs the enemy script byte helper func_801E2314 for class argument
 * 4, then runs the enemy readiness helper selected by the current enemy work
 * word +0x100: bit 0x10 takes enemyReadyOrHelper1, otherwise bit 0x20 takes
 * enemyReadyOrHelper2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E5288(void) {
  u32 work_flags;

  func_801E2314(4);
  work_flags = BATTLE_ENEMY_WORD_100(BATTLE_CURRENT_ENEMY_PTR);
  if ((work_flags & 0x10u) != 0u) {
    enemyReadyOrHelper1();
  } else if ((work_flags & 0x20u) != 0u) {
    enemyReadyOrHelper2();
  }
}
