#include "internal.h"

/* does: scans ids `3..10` and returns the unblocked id with the smallest enemy
 * halfword at offset `0x94`.
 * @source: 0x801e2ca4 FUN_801e2ca4
 */
u8 func_801e2ca4(void) {
  u8  result;
  u16 best;
  u8  slot;

  result = 0u;
  best = 0xffffu;
  slot = 3u;
  do {
    volatile Battle03EnemyWork* battle_work;

    battle_work = &BATTLE_ENEMY_WORK_ARRAY[slot - 3u];
    if ((func_801db524(slot) == 0u) &&
        (BATTLE_ENEMY_HALF_94(battle_work) < best)) {
      best = BATTLE_ENEMY_HALF_94(battle_work);
      result = slot;
    }
    slot += 1u;
  } while (slot < 0x0bu);

  return result;
}
