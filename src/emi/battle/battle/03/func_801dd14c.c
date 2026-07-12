#include "internal.h"

/* @behavior clears the small local/enemy action scratch flags for one battler and
 * removes it from the ranked owner list if present.
 * @source 0x801dd14c FUN_801dd14c
 */
void func_801dd14c(u8 arg0) {
  u8 index;

  if (arg0 < 3u) {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[arg0];
    BATTLE_LOCAL_BYTE_119(battle_work) = 0u;
    BATTLE_LOCAL_WORD_124(battle_work) &= 0xfffffffdu;
    BATTLE_LOCAL_WORD_128(battle_work) &= 0xfffffffbu;
  } else {
    volatile Battle03EnemyWork* battle_work;

    battle_work = &BATTLE_ENEMY_WORK_ARRAY[(arg0 - 3u) & 0xffu];
    BATTLE_ENEMY_BYTE_F5(battle_work) = 0u;
    BATTLE_ENEMY_WORD_100(battle_work) &= 0xfffffffdu;
  }

  index = 0u;
  while (index < BATTLE_GLOBAL_BYTE_6323) {
    if (arg0 == BATTLE_GLOBAL_BYTE_630C(index)) {
      BATTLE_GLOBAL_BYTE_630C(index) = 0xffu;
    }
    index += 1u;
  }
}
