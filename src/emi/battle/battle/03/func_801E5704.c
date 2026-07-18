#include "internal.h"

/* @behavior conditionally raises the global `0x80` bit and allocates one queued
 * slot when the current enemy scratch flags allow it.
 * @source 0x801E5704
 */
void func_801E5704(void) {
  if (((BATTLE_ENEMY_FLAGS_80(BATTLE_CURRENT_ENEMY_PTR) & 8u) != 0u) &&
      ((func_8017E3D4() & 1u) != 0u)) {
    BATTLE_GLOBAL_HALF_62E8 |= 0x80u;
    func_801E590C(0u, 2u);
  }
}
