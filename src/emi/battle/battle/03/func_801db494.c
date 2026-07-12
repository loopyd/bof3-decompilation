#include "internal.h"

/* @behavior clears one local/enemy ranking scratch set across all battlers.
 * @source 0x801db494 FUN_801db494
 */
void func_801db494(void) {
  u8 index;

  index = 0u;
  while (index < 3u) {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[index];
    BATTLE_LOCAL_BYTE_119(battle_work) = 0u;
    BATTLE_LOCAL_WORD_124(battle_work) = 0u;
    index += 1u;
  }

  index = 0u;
  while (index < 8u) {
    volatile Battle03EnemyWork* battle_work;

    battle_work = &BATTLE_ENEMY_WORK_ARRAY[index];
    BATTLE_ENEMY_BYTE_F5(battle_work) = 0u;
    BATTLE_ENEMY_WORD_100(battle_work) = 0u;
    index += 1u;
  }
}
