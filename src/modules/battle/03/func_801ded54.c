#include "internal.h"

/* @behavior scans the three local work records and runs the shared handler for each
 * active entry that does not have flag `0x40` set.
 * @source 0x801ded54 FUN_801ded54
 */
void func_801ded54(void) {
  u8 index;

  index = 0;
  do {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[index];
    BATTLE_LOCAL_WORK_PTR = battle_work;
    BATTLE_LOCAL_SCRATCH_PTR = battle_work;

    if (((battle_work->flags_00 & 1u) != 0u) &&
        ((battle_work->flags_00 & 0x40u) == 0u)) {
      func_8014d290();
    }

    index += 1u;
  } while (index < 3u);
}
