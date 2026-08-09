#include "bof3/battle/battle03_internal.h"

/* @behavior resets the current enemy scratch object's first two state bytes, clears
 * one pending bit, and conditionally clears byte `0xf5` on the current enemy.
 * @source 0x801E567C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void resetEnemyScratchState(void) {
  volatile Battle03EnemyWork **scratch = SPAD_PTR_TABLE(volatile Battle03EnemyWork);

  scratch[0x11]->unk_01 = 2u;
  BATTLE_ENEMY_BYTE_02(scratch[0x11]) = 0u;
  clearPendingBit(BATTLE_ENEMY_BYTE_05(scratch[0x11]));
  BATTLE_ENEMY_WORD_100(BATTLE_CURRENT_ENEMY_PTR) &= 0xfffffdffu;
  if ((BATTLE_GLOBAL_HALF_62E8 & 0x40u) == 0u) {
    BATTLE_ENEMY_BYTE_F5(BATTLE_CURRENT_ENEMY_PTR) = 0u;
  }
}
