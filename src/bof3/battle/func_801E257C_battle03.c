#include "bof3/battle/battle03_internal.h"

/* @source 0x801E257C
 * @behavior publishes the 0x118-byte work record at D_801EB2E8[index] as the
 * current enemy work, runs the enemy script byte helper func_801E2314 for the
 * masked argument, then restores the previously published record.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E257C(u8 index, u8 arg1) {
  Battle03EnemyWork* previous;

  previous = battleCurrentEnemyWorkState;
  /* D_801EB2E8 holds 0x118-byte records, so index * 0x118 == this shift chain. */
  battleCurrentEnemyWorkState =
      (Battle03EnemyWork*)(D_801EB2E8 + ((index * 36u - index) * 8u));
  func_801E2314(arg1);
  battleCurrentEnemyWorkState = previous;
}
