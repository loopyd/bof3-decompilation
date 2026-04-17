#include "internal.h"

/* does: initializes the deferred local/enemy halfword countdown from the
 * current primary value when bit `0x80` is set.
 * @source: 0x801dcef8 FUN_801dcef8
 */
void func_801dcef8(u32 arg0) {
  arg0 &= 0xffu;
  if (arg0 < 3u) {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BOF3_BATTLE_LOCAL_WORK_ARRAY[arg0];
    if ((BOF3_BATTLE_LOCAL_FLAGS_80(battle_work) & 0x80u) != 0u) {
      BOF3_BATTLE_LOCAL_HALF_1C(battle_work) =
          (BOF3_BATTLE_LOCAL_HALF_88(battle_work) + 5u) / 10u;
    }
  } else {
    volatile Battle03EnemyWork* battle_work;

    battle_work = &BOF3_BATTLE_ENEMY_WORK_ARRAY[(arg0 - 3u) & 0xffu];
    if ((BOF3_BATTLE_ENEMY_FLAGS_82(battle_work) & 0x80u) != 0u) {
      BOF3_BATTLE_ENEMY_HALF_F8(battle_work) =
          (BOF3_BATTLE_ENEMY_HALF_A0(battle_work) + 5u) / 10u;
    }
  }
}
