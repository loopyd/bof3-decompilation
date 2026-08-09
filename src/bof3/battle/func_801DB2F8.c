#include "bof3/battle/battle03_internal.h"

/* @behavior reports whether one battler slot is minimally eligible for the ranking
 * path, using only the active bit and the base local/enemy rejection masks.
 * @source 0x801DB2F8
 * @status partial
 * @match 31.11
 * @residual non-exact live audit: 14/42 instructions; 168 original bytes versus 180 current.
 */
u8 func_801DB2F8(u32 arg0) {
  if (arg0 < 3u) {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[arg0];
    if ((battle_work->flags_00 & 1u) == 0u) {
      return 0u;
    }
    return (BATTLE_LOCAL_FLAGS_80(battle_work) & 0x4944u) == 0u;
  }

  arg0 = (arg0 - 3u) & 0xffu;
  if ((BATTLE_ENEMY_WORK_ARRAY[arg0].unk_00 & 1u) == 0u) {
    return 0u;
  }
  return (BATTLE_ENEMY_FLAGS_82(&BATTLE_ENEMY_WORK_ARRAY[arg0]) & 0x4144u) ==
         0u;
}
