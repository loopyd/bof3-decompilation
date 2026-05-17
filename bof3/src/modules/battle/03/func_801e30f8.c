#include "internal.h"

/* does: returns ready immediately when the current enemy work flags allow it;
 * otherwise delegates to the first EXE-side readiness helper.
 * @source: 0x801e30f8 FUN_801e30f8
 */
u8 func_801e30f8(void) {
  u8 ready;

  ready = 1u;
  if (((BATTLE_ENEMY_FLAGS_82(BATTLE_CURRENT_ENEMY_PTR) & 0x44u) ==
       0u) &&
      ((BATTLE_LOCAL_FLAG_63CE == 0u) ||
       ((BATTLE_ENEMY_WORD_104(BATTLE_CURRENT_ENEMY_PTR) & 0x10u) !=
        0u))) {
    ready = func_8014d978();
  }

  return ready;
}
