#include "internal.h"

/* @behavior reports whether the current enemy work satisfies the stricter queued
 * predicate under the current global mode bytes.
 * @source 0x801e4368 FUN_801e4368
 */
u8 func_801e4368(void) {
  if ((BATTLE_ENEMY_FLAGS_82(BATTLE_CURRENT_ENEMY_PTR) & 0x4064u) != 0u) {
    return 0u;
  }
  if (BATTLE_GLOBAL_BYTE_6374 < 3u) {
    if ((*(u8*)&BATTLE_GLOBAL_BYTE_6375 == 4u) &&
        (*(u16*)&BATTLE_GLOBAL_HALF_63C0 == 0xa1u)) {
      return 0u;
    }
    if ((BATTLE_GLOBAL_BYTE_63CE != 0u) &&
        ((BATTLE_ENEMY_WORD_104(BATTLE_CURRENT_ENEMY_PTR) & 0x10u) == 0u)) {
      return 0u;
    }
    if ((BATTLE_ENEMY_FLAGS_80(BATTLE_CURRENT_ENEMY_PTR) & 2u) == 0u) {
      return 0u;
    }
    if ((BATTLE_ENEMY_WORD_100(BATTLE_CURRENT_ENEMY_PTR) & 0x8000u) != 0u) {
      return 1u;
    }
    if ((((s32 (*)(void))func_8017e3d4)() % 100) < 0x46) {
      return 1u;
    }
  }
  return 0u;
}
