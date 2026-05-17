#include "internal.h"

/* does: resets the current enemy scratch object's first two state bytes, clears
 * one pending bit, and conditionally clears byte `0xf5` on the current enemy.
 * @source: 0x801e567c FUN_801e567c
 */
void func_801e567c(void) {
  BATTLE_ENEMY_SCRATCH_PTR->unk_01 = 2u;
  BATTLE_ENEMY_BYTE_02(BATTLE_ENEMY_SCRATCH_PTR) = 0u;
  func_801de1b0(BATTLE_ENEMY_BYTE_05(BATTLE_ENEMY_SCRATCH_PTR));
  BATTLE_ENEMY_WORD_100(BATTLE_CURRENT_ENEMY_PTR) &= 0xfffffdffu;
  if ((BATTLE_GLOBAL_HALF_62E8 & 0x40u) == 0u) {
    BATTLE_ENEMY_BYTE_F5(BATTLE_CURRENT_ENEMY_PTR) = 0u;
  }
}
