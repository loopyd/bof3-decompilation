#include "internal.h"

/* does: advances one local `0x20` countdown path, clearing the flag and queuing
 * a followup event once one or more battlers complete.
 * @source: 0x801d590c FUN_801d590c
 */
u8 func_801d590c(void) {
  u8 index;
  u8 count;
  u8 last_index;

  count = 0u;
  last_index = 0u;
  index = 0u;
  do {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BOF3_BATTLE_LOCAL_WORK_ARRAY[index];
    if ((func_801d64c4(index) == 0u) &&
        ((BOF3_BATTLE_LOCAL_FLAGS_80(battle_work) & 0x20u) != 0u)) {
      if (func_801ddcb4(index) == 0u) {
        BOF3_BATTLE_LOCAL_BYTE_21(battle_work) += 1u;
      } else {
        BOF3_BATTLE_LOCAL_BYTE_21(battle_work) = 0u;
        BOF3_BATTLE_LOCAL_FLAGS_80(battle_work) &= 0xffdfu;
        count += 1u;
        last_index = index;
      }
    }
    index += 1u;
  } while (index < 3u);

  if (count != 0u) {
    u32 event_id;

    func_801d9484();
    event_id = 0x29u;
    if (count == 1u) {
      func_801de9a8(last_index);
      event_id = 0x28u;
    }
    event_id = func_801502d0(event_id);
    func_801de560(2u, 0u, 0u, 0x2du, event_id);
    return 1u;
  }

  return 0u;
}
