#include "internal.h"

/* @behavior advances one local state-0x800 countdown path, restores saved local
 * coordinates when it completes, and emits one followup event when any battler
 * finished.
 * @source 0x801d54f8 FUN_801d54f8
 */
u8 func_801d54f8(void) {
  u8 index;
  u8 count;
  u8 last_index;

  count = 0u;
  last_index = 0u;
  index = 0u;
  do {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[index];
    if ((func_801d64c4(index) == 0u) &&
        ((BATTLE_LOCAL_FLAGS_80(battle_work) & 0x0800u) != 0u) &&
        (BATTLE_LOCAL_BYTE_137(battle_work) == 5u)) {
      count += 1u;
      BATTLE_LOCAL_BYTE_137(battle_work) = 0u;
      BATTLE_LOCAL_HALF_88(battle_work) = BATTLE_LOCAL_HALF_90(battle_work);
      BATTLE_LOCAL_HALF_8A(battle_work) = BATTLE_LOCAL_HALF_92(battle_work);
      BATTLE_LOCAL_BYTE_8C(battle_work) = BATTLE_LOCAL_BYTE_9E(battle_work);
      func_800a36f0(index, 0x8ffu);
      last_index = index;
    }
    index += 1u;
  } while (index < 3u);

  if (count != 0u) {
    u32 event_id;

    func_801d9484();
    event_id = 0x2cu;
    if (count == 1u) {
      func_801de9a8(last_index);
      event_id = 0x2bu;
    }
    event_id = func_801502d0(event_id);
    func_801de560(2u, 0u, 0u, 0x2du, event_id);
    return 1u;
  }

  return 0u;
}
