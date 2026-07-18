#include "internal.h"

/* @behavior advances several battler-local countdown bytes and, when the global
 * suppression countdown expires, clears the shared `0x10` flag across all
 * currently available battlers.
 * @source 0x801D4D44
 */
void func_801D4D44(void) {
  u8 index;

  index = 0u;
  do {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[index];
    if (func_801D64C4(index) == 0u) {
      if (((battle_work->flags_00 & 1u) != 0u) &&
          (BATTLE_LOCAL_BYTE_136(battle_work) < 6u)) {
        BATTLE_LOCAL_BYTE_136(battle_work) += 1u;
      }
      if (((BATTLE_LOCAL_FLAGS_80(battle_work) & 0x0800u) != 0u) &&
          (BATTLE_LOCAL_BYTE_137(battle_work) < 6u)) {
        BATTLE_LOCAL_BYTE_137(battle_work) += 1u;
      }
      if (((BATTLE_LOCAL_WORD_128(battle_work) & 0x4000u) != 0u) &&
          (BATTLE_LOCAL_BYTE_136(battle_work) < 3u)) {
        BATTLE_LOCAL_BYTE_136(battle_work) += 1u;
      }
    }
    index += 1u;
  } while (index < 3u);

  index = 3u;
  do {
    volatile Battle03EnemyWork* battle_work;

    battle_work = &BATTLE_ENEMY_WORK_ARRAY[index - 3u];
    if ((func_801D64C4(index) == 0u) &&
        ((BATTLE_ENEMY_WORD_104(battle_work) & 0x4000u) != 0u) &&
        (BATTLE_ENEMY_BYTE_112(battle_work) < 3u)) {
      BATTLE_ENEMY_BYTE_112(battle_work) += 1u;
    }
    index += 1u;
  } while (index < 0x0bu);

  if (BATTLE_GLOBAL_BYTE_63CE != 0u) {
    BATTLE_GLOBAL_BYTE_63CE -= 1u;
    if (BATTLE_GLOBAL_BYTE_63CE == 0u) {
      index = 0u;
      do {
        volatile Battle03LocalWork* battle_work;

        battle_work = &BATTLE_LOCAL_WORK_ARRAY[index];
        if (func_801DB524(index) == 0u) {
          BATTLE_LOCAL_WORD_128(battle_work) &= 0xffffffefu;
        }
        index += 1u;
      } while (index < 3u);

      index = 3u;
      do {
        volatile Battle03EnemyWork* battle_work;

        battle_work = &BATTLE_ENEMY_WORK_ARRAY[index - 3u];
        if (func_801DB524(index) == 0u) {
          BATTLE_ENEMY_WORD_104(battle_work) &= 0xffffffefu;
        }
        index += 1u;
      } while (index < 0x0bu);
    }
  }
}
