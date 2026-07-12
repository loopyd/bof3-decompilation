#include "internal.h"

/* @behavior reports whether one battler slot is action-eligible under the current
 * owner mode byte, using the stricter local/enemy rejection mask set.
 * @source 0x801db844 FUN_801db844
 */
u8 func_801db844(u32 arg0) {
  if (arg0 < 3u) {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[arg0];
    if ((battle_work->flags_00 & 1u) == 0u) {
      return 0u;
    }
    if ((BATTLE_LOCAL_FLAGS_80(battle_work) & 0x4964u) != 0u) {
      return 0u;
    }
    if ((BATTLE_GLOBAL_BYTE_63CE != 0u) &&
        ((BATTLE_LOCAL_WORD_128(battle_work) & 0x10u) == 0u)) {
      return 0u;
    }
    if ((BATTLE_LOCAL_WORD_128(battle_work) & 0x14001u) != 0u) {
      return 0u;
    }
    return (BATTLE_GLOBAL_BYTE_6324 != 2u) && (BATTLE_GLOBAL_BYTE_6324 != 3u);
  }

  arg0 = (arg0 - 3u) & 0xffu;
  if ((BATTLE_ENEMY_WORK_ARRAY[arg0].unk_00 & 1u) == 0u) {
    return 0u;
  }
  if ((BATTLE_ENEMY_FLAGS_82(&BATTLE_ENEMY_WORK_ARRAY[arg0]) & 0x4164u) != 0u) {
    return 0u;
  }
  if ((BATTLE_GLOBAL_BYTE_63CE != 0u) &&
      ((BATTLE_ENEMY_WORD_104(&BATTLE_ENEMY_WORK_ARRAY[arg0]) & 0x10u) == 0u)) {
    return 0u;
  }
  if ((BATTLE_ENEMY_WORD_104(&BATTLE_ENEMY_WORK_ARRAY[arg0]) & 0x4000u) != 0u) {
    return 0u;
  }

  return (BATTLE_GLOBAL_BYTE_6324 != 1u) && (BATTLE_GLOBAL_BYTE_6324 != 3u);
}
