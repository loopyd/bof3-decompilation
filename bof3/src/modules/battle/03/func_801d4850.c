#include "internal.h"

/* does: clears paired local/enemy flag bits and associated countdown bytes for
 * the current battler, except under two specific global mode/kind combinations.
 * @source: 0x801d4850 FUN_801d4850
 */
void func_801d4850(void) {
  if ((BOF3_BATTLE_GLOBAL_BYTE_6375 != 4u) ||
      (BOF3_BATTLE_GLOBAL_HALF_63C0 != 0x27u)) {
    if (BOF3_BATTLE_GLOBAL_BYTE_6374 < 3u) {
      volatile Battle03LocalWork* battle_work;

      battle_work = &BOF3_BATTLE_LOCAL_WORK_ARRAY[BOF3_BATTLE_GLOBAL_BYTE_6374];
      if ((BOF3_BATTLE_LOCAL_WORD_128(battle_work) & 0x40u) != 0u) {
        BOF3_BATTLE_LOCAL_BYTE_138(battle_work) = 0u;
        BOF3_BATTLE_LOCAL_WORD_128(battle_work) &= 0xffffffbfu;
      }
    } else {
      volatile Battle03EnemyWork* battle_work;

      battle_work =
          &BOF3_BATTLE_ENEMY_WORK_ARRAY[BOF3_BATTLE_GLOBAL_BYTE_6374 - 3u];
      if ((BOF3_BATTLE_ENEMY_WORD_104(battle_work) & 0x40u) != 0u) {
        BOF3_BATTLE_ENEMY_BYTE_114(battle_work) = 0u;
        BOF3_BATTLE_ENEMY_WORD_104(battle_work) &= 0xffffffbfu;
      }
    }
  }

  if ((BOF3_BATTLE_GLOBAL_BYTE_6375 != 4u) ||
      (BOF3_BATTLE_GLOBAL_HALF_63C0 != 0xa3u)) {
    if (BOF3_BATTLE_GLOBAL_BYTE_6374 < 3u) {
      volatile Battle03LocalWork* battle_work;

      battle_work = &BOF3_BATTLE_LOCAL_WORK_ARRAY[BOF3_BATTLE_GLOBAL_BYTE_6374];
      if ((BOF3_BATTLE_LOCAL_WORD_128(battle_work) & 0x80u) != 0u) {
        BOF3_BATTLE_LOCAL_BYTE_139(battle_work) = 0u;
        BOF3_BATTLE_LOCAL_WORD_128(battle_work) &= 0xffffff7fu;
      }
    } else {
      volatile Battle03EnemyWork* battle_work;

      battle_work =
          &BOF3_BATTLE_ENEMY_WORK_ARRAY[BOF3_BATTLE_GLOBAL_BYTE_6374 - 3u];
      if ((BOF3_BATTLE_ENEMY_WORD_104(battle_work) & 0x80u) != 0u) {
        BOF3_BATTLE_ENEMY_BYTE_115(battle_work) = 0u;
        BOF3_BATTLE_ENEMY_WORD_104(battle_work) &= 0xffffff7fu;
      }
    }
  }
}
