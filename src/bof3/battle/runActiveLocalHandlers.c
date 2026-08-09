#include "bof3/battle/battle03_internal.h"

/* @behavior scans the three local work records and runs the shared handler for each
 * active entry that does not have flag `0x40` set.
 * @source 0x801DED54
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void runActiveLocalHandlers(void) {
  u8 index;

  index = 0;
  do {
    Battle03LocalWork* battle_work;

    battle_work = &D_80145E90[index];
    BATTLE_LOCAL_WORK_PTR = battle_work;
    BATTLE_LOCAL_SCRATCH_PTR = battle_work;

    if (((battle_work->flags_00 & 1u) != 0u) &&
        ((battle_work->flags_00 & 0x40u) == 0u)) {
      func_8014D290();
    }

    index += 1u;
  } while (index < 3u);
}
