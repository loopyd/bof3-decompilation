#include "internal.h"

/* @behavior resets one local/enemy `0x80` path, zeroes saved values, rebuilds the
 * followup state bytes, and queues the common event when needed.
 * @source 0x801D5BC0
 */
u8 func_801D5BC0(void) {
  u8 index;
  u8 count;

  count = 0u;
  index = 0u;
  do {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[index];
    if ((func_801D64C4(index) == 0u) &&
        ((BATTLE_LOCAL_FLAGS_80(battle_work) & 0x80u) != 0u)) {
      BATTLE_LOCAL_HALF_1E(battle_work) = 0u;
      BATTLE_LOCAL_HALF_1C(battle_work) = 0u;
      func_801DCEF8(index);
      count += 1u;
      battle_work->unk_01 = 6u;
      battle_work->unk_02 = 5u;
      battle_work->unk_04 = 0u;
      battle_work->unk_03 = 0u;
      battle_work->unk_20 = 0x11u;
      markPendingBit(index);
    }
    index += 1u;
  } while (index < 3u);

  index = 3u;
  do {
    volatile Battle03EnemyWork* battle_work;

    battle_work = &BATTLE_ENEMY_WORK_ARRAY[index - 3u];
    if ((func_801D64C4(index) == 0u) &&
        ((BATTLE_ENEMY_FLAGS_82(battle_work) & 0x80u) != 0u)) {
      BATTLE_ENEMY_HALF_FA(battle_work) = 0u;
      BATTLE_ENEMY_HALF_F8(battle_work) = 0u;
      func_801DCEF8(index);
      count += 1u;
      battle_work->unk_01 = 6u;
      BATTLE_ENEMY_BYTE_02(battle_work) = 5u;
      BATTLE_ENEMY_BYTE_04(battle_work) = 0u;
      BATTLE_ENEMY_BYTE_03(battle_work) = 0u;
      BATTLE_ENEMY_BYTE_FC(battle_work) = 0x11u;
      markPendingBit(index);
    }
    index += 1u;
  } while (index < 0x0bu);

  if (count != 0u) {
    u32 event_id;

    initUiBundleSlot0();
    event_id = func_801502D0(0x18u);
    func_801DE560(2u, 0u, 0u, 0x2du, event_id);
    return 1u;
  }

  return 0u;
}
