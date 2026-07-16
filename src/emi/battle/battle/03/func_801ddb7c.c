#include "internal.h"

/* @behavior walks active local and enemy battlers, reserving one queued slot for
 * each and copying its current state record into that slot.
 * @source 0x801DDB7C
 */
void func_801DDB7C(void) {
  u8 index;

  index = 0u;
  while (index < 3u) {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[index];
    if ((battle_work->flags_00 & 1u) == 0u) {
      func_801E590C(0u, 0u);
    } else {
      BATTLE_LOCAL_SCRATCH_PTR = battle_work;
      BATTLE_LOCAL_WORK_PTR = battle_work;
      func_801DDAF0();
    }
    index += 1u;
  }

  index = 3u;
  while (index < 0x0bu) {
    volatile Battle03EnemyWork* battle_work;

    battle_work = &BATTLE_ENEMY_WORK_ARRAY[index - 3u];
    if ((battle_work->unk_00 & 1u) == 0u) {
      func_801E590C(0u, 0u);
    } else {
      BATTLE_ENEMY_SCRATCH_PTR = battle_work;
      BATTLE_CURRENT_ENEMY_PTR = battle_work;
      func_801DDAF0();
    }
    index += 1u;
  }
}
