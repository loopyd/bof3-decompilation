#include "bof3/battle/battle03_internal.h"

/* @source 0x801E2284
 * @behavior Scans the eight 0x118-byte reserve work records at D_801EB630 and
 * publishes the first occupied record as the current enemy work (scratchpad
 * +0x44 and the 0x801EB4E8 global), then runs func_8014D290; empty records are
 * skipped and nothing is published or called when all eight are empty.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E2284(void) {
  u8                 index;
  Battle03EnemyWork* work;

  for (index = 0; index < 8; index++) {
    if (D_801EB630[index].unk_00 == 0) {
      continue;
    }
    work = &D_801EB630[index];
    BATTLE_ENEMY_SCRATCH_PTR = work;
    battleCurrentEnemyWorkState = work;
    func_8014D290();
  }
}
