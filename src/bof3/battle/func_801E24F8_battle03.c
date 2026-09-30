#include "bof3/battle/battle03_internal.h"

/* @source 0x801E24F8
 * @behavior Publishes the 0x118-byte work record at D_801EB2E8[index] as the
 * current enemy work -- both the scratchpad +0x44 cell and the 0x801EB4E8
 * global -- runs the enemy script byte helper func_801E2314 for the masked
 * argument, then restores both previously published pointers.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E24F8(u8 index, u8 arg1) {
  volatile u8*       previousWork;
  Battle03EnemyWork* previousEnemy;
  Battle03EnemyWork* record;

  previousWork = battleWork;
  previousEnemy = battleCurrentEnemyWorkState;
  /* D_801EB2E8 holds 0x118-byte records, so index * 0x118 == this shift chain. */
  record = (Battle03EnemyWork*)(D_801EB2E8 + ((index * 36u - index) * 8u));
  battleWork = (u8*)record;
  battleCurrentEnemyWorkState = record;
  func_801E2314(arg1);
  battleWork = previousWork;
  battleCurrentEnemyWorkState = previousEnemy;
}
