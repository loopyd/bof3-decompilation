#include "internal.h"

/* does: computes average/max thresholds for eligible enemy and local battlers,
 * then marks the battlers that pass each threshold pair with the shared
 * `0x8000` state bit.
 * @source: 0x801db058 FUN_801db058
 */
u8 func_801db058(void) {
  u16 total;
  u16 maximum;
  u8  count;
  u8  index;
  u8  local_hits;

  total = 0u;
  maximum = 0u;
  count = 0u;
  index = 3u;
  while (index < 0x0bu) {
    volatile Battle03EnemyWork* battle_work;

    battle_work = &BATTLE_ENEMY_WORK_ARRAY[index - 3u];
    if (func_801db2f8(index) != 0u) {
      total += BATTLE_ENEMY_HALF_A8(battle_work);
      if (maximum < BATTLE_ENEMY_HALF_A8(battle_work)) {
        maximum = BATTLE_ENEMY_HALF_A8(battle_work);
      }
      count += 1u;
    }
    index += 1u;
  }

  if (total != 0u) {
    total = total / count;
  }

  local_hits = 0u;
  index = 0u;
  while (index < 3u) {
    if ((func_801db9e4(index) != 0u) &&
        (func_801db3a0(index, total, maximum) != 0u)) {
      BATTLE_LOCAL_WORD_128(&BATTLE_LOCAL_WORK_ARRAY[index]) |=
          0x8000u;
      local_hits += 1u;
    }
    index += 1u;
  }

  total = 0u;
  maximum = 0u;
  count = 0u;
  index = 0u;
  while (index < 3u) {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[index];
    if (func_801db2f8(index) != 0u) {
      total += BATTLE_LOCAL_HALF_98(battle_work);
      if (maximum < BATTLE_LOCAL_HALF_98(battle_work)) {
        maximum = BATTLE_LOCAL_HALF_98(battle_work);
      }
      count += 1u;
    }
    index += 1u;
  }

  if (total != 0u) {
    total = total / count;
  }

  index = 3u;
  while (index < 0x0bu) {
    if ((func_801db9e4(index) != 0u) &&
        (func_801db3e4(index, total, maximum) != 0u)) {
      BATTLE_ENEMY_WORD_104(&BATTLE_ENEMY_WORK_ARRAY[index - 3u]) |=
          0x8000u;
    }
    index += 1u;
  }

  return local_hits != 0u;
}
