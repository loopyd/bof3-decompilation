#include "internal.h"

/* does: advances one enemy `0x20` countdown path, clearing the flag and queuing
 * a followup event once one or more battlers complete.
 * @source: 0x801d5a60 FUN_801d5a60
 */
u8 func_801d5a60(void) {
  u8 index;
  u8 count;
  u8 last_index;

  count = 0u;
  last_index = 0u;
  index = 3u;
  do {
    volatile Battle03EnemyWork* battle_work;

    battle_work = &BOF3_BATTLE_ENEMY_WORK_ARRAY[index - 3u];
    if ((func_801d64c4(index) == 0u) &&
        ((BOF3_BATTLE_ENEMY_FLAGS_82(battle_work) & 0x20u) != 0u)) {
      if (func_801ddcb4(index) == 0u) {
        BOF3_BATTLE_ENEMY_BYTE_FD(battle_work) += 1u;
      } else {
        BOF3_BATTLE_ENEMY_BYTE_FD(battle_work) = 0u;
        BOF3_BATTLE_ENEMY_FLAGS_82(battle_work) &= 0xffdfu;
        count += 1u;
        last_index = index;
      }
    }
    index += 1u;
  } while (index < 0x0bu);

  if (count != 0u) {
    u32 event_id;

    func_801d9484();
    event_id = 0x2au;
    if (count == 1u) {
      func_801dea18(last_index);
      event_id = 0x28u;
    }
    event_id = func_801502d0(event_id);
    func_801de560(2u, 0u, 0u, 0x2du, event_id);
    return 1u;
  }

  return 0u;
}
