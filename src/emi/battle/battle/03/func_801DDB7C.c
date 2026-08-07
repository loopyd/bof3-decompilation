#include "internal.h"

/* @behavior walks active local and enemy battlers, reserving one queued slot for
 * each and copying its current state record into that slot.
 * @source 0x801DDB7C
 */
void func_801DDB7C(void) {
  u8 index;

  index = 0u;
  while (index < 3u) {
    if ((D_80145E90[index].flags_00 & 1u) != 0u) {
      Battle03LocalWork* battle_work = &D_80145E90[index];

      BATTLE_LOCAL_SCRATCH_PTR = battle_work;
      BATTLE_LOCAL_WORK_PTR = battle_work;
      func_801DDAF0();
    } else {
      func_801E590C(0u, 0u);
    }
    index += 1u;
  }

  index = 3u;
  while (index < 0x0bu) {
    if ((D_801EB630[index - 3u].unk_00 & 1u) != 0u) {
      Battle03EnemyWork* battle_work = &D_801EB630[index - 3u];

      BATTLE_ENEMY_SCRATCH_PTR = battle_work;
      BATTLE_CURRENT_ENEMY_PTR = battle_work;
      func_801DDAF0();
    } else {
      func_801E590C(0u, 0u);
    }
    index += 1u;
  }
}
