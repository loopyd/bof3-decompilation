#include "internal.h"

/* @behavior initializes the deferred local/enemy halfword countdown from the
 * current primary value when bit `0x80` is set.
 * @source 0x801DCEF8
 */
void func_801DCEF8(u32 arg0) {
  arg0 &= 0xffu;
  if (arg0 < 3u) {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[arg0];
    if ((BATTLE_LOCAL_FLAGS_80(battle_work) & 0x80u) != 0u) {
      BATTLE_LOCAL_HALF_1C(battle_work) =
          (BATTLE_LOCAL_HALF_88(battle_work) + 5u) / 10u;
    }
  } else {
    volatile Battle03EnemyWork* battle_work;

    battle_work = &BATTLE_ENEMY_WORK_ARRAY[(arg0 - 3u) & 0xffu];
    if ((BATTLE_ENEMY_FLAGS_82(battle_work) & 0x80u) != 0u) {
      BATTLE_ENEMY_HALF_F8(battle_work) =
          (BATTLE_ENEMY_HALF_A0(battle_work) + 5u) / 10u;
    }
  }
}
