#include "internal.h"

/* does: rebuilds one local/enemy movement delta from several battler-state
 * bytes, then converts any non-zero result into the common queued followup
 * state.
 * @source: 0x801d5dcc FUN_801d5dcc
 */
u8 func_801d5dcc(void) {
  u8 index;

  index = 0u;
  do {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[index];
    if (func_801d64c4(index) == 0u) {
      BATTLE_LOCAL_HALF_1C(battle_work) = 0u;
      BATTLE_LOCAL_HALF_1E(battle_work) = 0u;
      if (BATTLE_GLOBAL_BYTE_44F58 == 5u) {
        BATTLE_LOCAL_HALF_1C(battle_work) -= 1u;
      }
      if (BATTLE_LOCAL_BYTE_79(battle_work) == 6u) {
        BATTLE_LOCAL_HALF_1C(battle_work) -=
            (BATTLE_LOCAL_HALF_90(battle_work) + 10u) / 0x14u;
      }
      if (((BATTLE_LOCAL_BYTE_79(battle_work) == 7u) ||
           (BATTLE_LOCAL_BYTE_79(battle_work) == 0u)) &&
          ((BATTLE_LOCAL_WORD_128(battle_work) & 2u) != 0u) &&
          (BATTLE_GLOBAL_BYTE_63C9 == 0x12u)) {
        BATTLE_LOCAL_HALF_1C(battle_work) -=
            (BATTLE_LOCAL_HALF_90(battle_work) + 10u) / 0x14u;
      }
      if (BATTLE_LOCAL_BYTE_82(battle_work) == 'R') {
        BATTLE_LOCAL_HALF_1C(battle_work) +=
            (BATTLE_LOCAL_HALF_90(battle_work) + 10u) / 0x14u;
        if ((u32)BATTLE_LOCAL_HALF_88(battle_work) <=
            (u32)BATTLE_LOCAL_HALF_1C(battle_work)) {
          BATTLE_LOCAL_HALF_1C(battle_work) =
              BATTLE_LOCAL_HALF_88(battle_work) - 1u;
        }
      }
      if (BATTLE_LOCAL_BYTE_85(battle_work) == 0x1fu) {
        BATTLE_LOCAL_HALF_1C(battle_work) -= 1u;
      }
      if (BATTLE_LOCAL_BYTE_86(battle_work) == 0x16u) {
        BATTLE_LOCAL_HALF_1C(battle_work) -= 1u;
      }
      if (BATTLE_LOCAL_BYTE_87(battle_work) == 0x16u) {
        BATTLE_LOCAL_HALF_1C(battle_work) -= 1u;
      }
      if (BATTLE_LOCAL_BYTE_86(battle_work) == 0x17u) {
        BATTLE_LOCAL_HALF_1C(battle_work) -= 1u;
      }
      if (BATTLE_LOCAL_BYTE_87(battle_work) == 0x17u) {
        BATTLE_LOCAL_HALF_1C(battle_work) -= 1u;
      }
      if ((BATTLE_LOCAL_FLAGS_80(battle_work) & 1u) != 0u) {
        BATTLE_LOCAL_HALF_1C(battle_work) -=
            BATTLE_LOCAL_HALF_90(battle_work) >> 1;
      }
      if (BATTLE_LOCAL_HALF_1C(battle_work) != 0u) {
        battle_work->unk_01 = 6u;
        battle_work->unk_02 = 5u;
        battle_work->unk_04 = 0u;
        battle_work->unk_03 = 0u;
        battle_work->unk_20 = 0x11u;
        func_801de190(index);
      }
    }
    index += 1u;
  } while (index < 3u);

  index = 3u;
  do {
    volatile Battle03EnemyWork* battle_work;

    battle_work = &BATTLE_ENEMY_WORK_ARRAY[index - 3u];
    if ((func_801d64c4(index) == 0u) &&
        ((BATTLE_ENEMY_FLAGS_82(battle_work) & 1u) != 0u)) {
      battle_work->unk_01 = 6u;
      BATTLE_ENEMY_BYTE_02(battle_work) = 5u;
      BATTLE_ENEMY_BYTE_04(battle_work) = 0u;
      BATTLE_ENEMY_BYTE_03(battle_work) = 0u;
      BATTLE_ENEMY_BYTE_FC(battle_work) = 0x11u;
      BATTLE_ENEMY_HALF_F8(battle_work) =
          -(BATTLE_ENEMY_HALF_A0(battle_work) >> 1);
      func_801de190(index);
    }
    index += 1u;
  } while (index < 0x0bu);

  return BATTLE_GLOBAL_HALF_63C2 != 0u;
}
