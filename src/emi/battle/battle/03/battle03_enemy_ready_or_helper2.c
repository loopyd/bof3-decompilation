#include "internal.h"

/* @behavior reports whether the current enemy work is immediately ready, otherwise
 * delegating to the second EXE-side readiness helper.
 * @source 0x801E3160
 */
u8 battle03_enemy_ready_or_helper2(void) {
  volatile Battle03EnemyWork* enemy =
    PSX_REF(volatile Battle03EnemyWork*, 0x801EB4E8u);

  if ((BATTLE_ENEMY_FLAGS_82(enemy) & 0x44u) != 0u) {
    return 1u;
  }
  if ((BATTLE_GLOBAL_BYTE_63CE != 0u) &&
      ((BATTLE_ENEMY_WORD_104(enemy) & 0x10u) == 0u)) {
    return 1u;
  }
  return func_8014DAEC();
}
