#include "internal.h"

/* @behavior reports whether one battler slot is display-eligible under the current
 * owner mode byte, using the lighter local/enemy rejection mask set.
 * @source 0x801db9e4 FUN_801db9e4
 */
u8 func_801db9e4(u32 arg0) {
  if (arg0 < 3u) {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[arg0];
    if ((battle_work->flags_00 & 1u) == 0u) {
      return 0u;
    }
    if ((BATTLE_LOCAL_FLAGS_80(battle_work) & 0x4944u) != 0u) {
      return 0u;
    }
    if ((BATTLE_GLOBAL_BYTE_63CE != 0u) &&
        ((BATTLE_LOCAL_WORD_128(battle_work) & 0x10u) == 0u)) {
      return 0u;
    }
    return (BATTLE_GLOBAL_BYTE_6324 != 2u) && (BATTLE_GLOBAL_BYTE_6324 != 3u);
  }

  arg0 = (arg0 - 3u) & 0xffu;
  if ((BATTLE_ENEMY_WORK_ARRAY[arg0].unk_00 & 1u) == 0u) {
    return 0u;
  }
  if ((BATTLE_ENEMY_FLAGS_82(&BATTLE_ENEMY_WORK_ARRAY[arg0]) & 0x4144u) != 0u) {
    return 0u;
  }
  if ((BATTLE_GLOBAL_BYTE_63CE != 0u) &&
      ((BATTLE_ENEMY_WORD_104(&BATTLE_ENEMY_WORK_ARRAY[arg0]) & 0x10u) == 0u)) {
    return 0u;
  }

  return (BATTLE_GLOBAL_BYTE_6324 != 1u) && (BATTLE_GLOBAL_BYTE_6324 != 3u);
}
