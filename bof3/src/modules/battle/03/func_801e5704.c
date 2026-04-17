#include "internal.h"

/* does: conditionally raises the global `0x80` bit and allocates one queued
 * slot when the current enemy scratch flags allow it.
 * @source: 0x801e5704 FUN_801e5704
 */
void func_801e5704(void) {
  if (((BOF3_BATTLE_ENEMY_FLAGS_80(BOF3_BATTLE_CURRENT_ENEMY_PTR) & 8u) !=
       0u) &&
      ((func_8017e3d4() & 1u) != 0u)) {
    BOF3_BATTLE_GLOBAL_HALF_62E8 |= 0x80u;
    func_801e590c(0u, 2u);
  }
}
