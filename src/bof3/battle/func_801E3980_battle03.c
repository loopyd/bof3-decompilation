#include "bof3/battle/battle03_internal.h"

/* @source 0x801E3980
 * @behavior Runs the enemy script byte helper func_801E2314 for class argument
 * 2, then publishes the 0x800E40CE class-table byte at stride 0x88 selected by
 * the current enemy work's byte +0xE0 as scratch work byte +0x09, increments
 * scratch work byte +0x01 and clears scratch work byte +0x02.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E3980(void) {
  u8 index;

  func_801E2314(2);
  index = BATTLE_ENEMY_BYTE_E0(battleCurrentEnemyWorkState);
  ((u8*)D_1F800044)[9] = BATTLE_CLASS_BYTE_0C0CE(index);
  ((u8*)D_1F800044)[1]++;
  ((u8*)D_1F800044)[2] = 0;
}
