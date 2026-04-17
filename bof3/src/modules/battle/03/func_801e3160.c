#include "internal.h"

/* does: reports whether the current enemy work is immediately ready, otherwise
 * delegating to the second EXE-side readiness helper.
 * @source: 0x801e3160 FUN_801e3160
 */
u8 func_801e3160(void) {
  if (((BOF3_BATTLE_ENEMY_FLAGS_82(BOF3_BATTLE_CURRENT_ENEMY_PTR) & 0x44u) ==
       0u) &&
      ((BOF3_BATTLE_GLOBAL_BYTE_63CE == 0u) ||
       ((BOF3_BATTLE_ENEMY_WORD_104(BOF3_BATTLE_CURRENT_ENEMY_PTR) & 0x10u) !=
        0u))) {
    return func_8014daec();
  }
  return 1u;
}
