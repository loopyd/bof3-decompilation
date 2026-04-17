#include "internal.h"

/* does: returns the unblocked local work index with the smallest halfword at
 * offset `0x88` across the first three local records.
 * @source: 0x801e2d90 FUN_801e2d90
 */
u8 func_801e2d90(void) {
  u8  result;
  u16 best;
  u8  slot;

  result = 0u;
  best = 10000u;
  slot = 0u;
  do {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BOF3_BATTLE_LOCAL_WORK_ARRAY[slot];
    if ((func_801db524(slot) == 0u) &&
        (BOF3_BATTLE_LOCAL_HALF_88(battle_work) < best)) {
      best = BOF3_BATTLE_LOCAL_HALF_88(battle_work);
      result = slot;
    }
    slot += 1u;
  } while (slot < 3u);

  return result;
}
