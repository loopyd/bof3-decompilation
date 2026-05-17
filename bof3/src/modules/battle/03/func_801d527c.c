#include "internal.h"

/* does: closes out one local/enemy `0x4000` path once its countdown reaches
 * three, rebuilds the followup state bytes, and queues the common event if any
 * battler transitioned this frame.
 * @source: 0x801d527c FUN_801d527c
 */
u8 func_801d527c(void) {
  u8 index;
  u8 count;

  count = 0u;
  index = 0u;
  do {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[index];
    if ((func_801d64c4(index) == 0u) &&
        ((BATTLE_LOCAL_WORD_128(battle_work) & 0x4000u) != 0u) &&
        (BATTLE_LOCAL_BYTE_136(battle_work) == 3u)) {
      count += 1u;
      battle_work->unk_01 = 6u;
      battle_work->unk_02 = 5u;
      battle_work->unk_20 = 0x11u;
      BATTLE_LOCAL_BYTE_136(battle_work) = 0u;
      battle_work->unk_04 = 0u;
      battle_work->unk_03 = 0u;
      BATTLE_LOCAL_WORD_128(battle_work) &= 0xffffbfffu;
      BATTLE_LOCAL_HALF_1C(battle_work) =
          BATTLE_LOCAL_HALF_88(battle_work);
      func_801de190(index);
    }
    index += 1u;
  } while (index < 3u);

  index = 3u;
  do {
    volatile Battle03EnemyWork* battle_work;

    battle_work = &BATTLE_ENEMY_WORK_ARRAY[index - 3u];
    if ((func_801d64c4(index) == 0u) &&
        ((BATTLE_ENEMY_WORD_104(battle_work) & 0x4000u) != 0u) &&
        (BATTLE_SLOT_BYTE_136(index) == 3u)) {
      count += 1u;
      battle_work->unk_01 = 6u;
      BATTLE_ENEMY_BYTE_02(battle_work) = 5u;
      BATTLE_ENEMY_BYTE_FC(battle_work) = 0x11u;
      BATTLE_ENEMY_BYTE_112(battle_work) = 0u;
      BATTLE_ENEMY_BYTE_04(battle_work) = 0u;
      BATTLE_ENEMY_BYTE_03(battle_work) = 0u;
      BATTLE_ENEMY_WORD_104(battle_work) &= 0xffffbfffu;
      BATTLE_ENEMY_HALF_F8(battle_work) =
          BATTLE_ENEMY_HALF_94(battle_work);
      func_801de190(index);
    }
    index += 1u;
  } while (index < 0x0bu);

  if (count != 0u) {
    u32 event_id;

    func_801d9484();
    event_id = func_801502d0(0x2du);
    func_801de560(2u, 0u, 0u, 0x2du, event_id);
    return 1u;
  }

  return 0u;
}
