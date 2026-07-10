#include "internal.h"

/* does: reports whether one local/enemy battler slot is unavailable, either
 * because it is inactive or because its corresponding `0x4000` flag is set.
 * @source: 0x801db524 FUN_801db524
 */
u8 func_801db524(u8 arg0) {
  if (arg0 < 3u) {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[arg0];
    if ((battle_work->flags_00 & 1u) == 0u) {
      return 1u;
    }
    return (BATTLE_LOCAL_FLAGS_80(battle_work) & 0x4000u) != 0u;
  }

  arg0 = (arg0 - 3u) & 0xffu;
  if ((BATTLE_ENEMY_WORK_ARRAY[arg0].unk_00 & 1u) == 0u) {
    return 1u;
  }
  return (BATTLE_ENEMY_FLAGS_82(&BATTLE_ENEMY_WORK_ARRAY[arg0]) & 0x4000u) !=
         0u;
}
