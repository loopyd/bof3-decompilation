#include "internal.h"

typedef struct Battle03RankLocalEntry {
  s16 value;
  u16 index;
} Battle03RankLocalEntry;

/* @behavior ranks the currently display-eligible local battlers by their `0x98`
 * value, then writes the surviving sorted indices into the three-byte owner
 * selection list at `0x801462f6`.
 * @source 0x801da7d4 FUN_801da7d4
 */
void func_801da7d4(void) {
  Battle03RankLocalEntry entries[3];
  u8                     count;
  u8                     index;
  u16                    max_enemy_value;

  max_enemy_value = 0u;
  index = 3u;
  while (index < 0x0bu) {
    volatile Battle03EnemyWork* battle_work;

    battle_work = &BATTLE_ENEMY_WORK_ARRAY[index - 3u];
    if ((BATTLE_GLOBAL_BYTE_63BA == 0u) ||
        ((BATTLE_ENEMY_WORD_104(battle_work) & 0x8000u) != 0u)) {
      if ((func_801db9e4(index) != 0u) &&
          (max_enemy_value < BATTLE_ENEMY_HALF_A8(battle_work))) {
        max_enemy_value = BATTLE_ENEMY_HALF_A8(battle_work);
      }
    }
    index += 1u;
  }

  count = 0u;
  index = 0u;
  while (index < 3u) {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[index];
    if ((BATTLE_GLOBAL_BYTE_63BA == 0u) ||
        ((BATTLE_LOCAL_WORD_128(battle_work) & 0x8000u) != 0u)) {
      if (func_801db844(index) != 0u) {
        entries[count].value = BATTLE_LOCAL_HALF_98(battle_work);
        entries[count].index = index;
        count += 1u;
      }
    }
    index += 1u;
  }

  if (count > 1u) {
    u8 left;

    left = 0u;
    while (left + 1u < count) {
      u8 right;

      right = left + 1u;
      while (right < count) {
        if (entries[left].value < entries[right].value) {
          Battle03RankLocalEntry temp;

          temp = entries[left];
          entries[left] = entries[right];
          entries[right] = temp;
        }
        right += 1u;
      }
      left += 1u;
    }
  }

  index = 0u;
  while (index < 3u) {
    BATTLE_GLOBAL_BYTE_62F6(index) = 0xffu;
    index += 1u;
  }

  BATTLE_GLOBAL_BYTE_6303 = 0u;
  index = 0u;
  while (index < count) {
    if (func_801db844(entries[index].index) != 0u) {
      BATTLE_GLOBAL_BYTE_62F6(BATTLE_GLOBAL_BYTE_6303) =
          (u8)entries[index].index;
      BATTLE_GLOBAL_BYTE_6303 += 1u;
    }
    index += 1u;
  }
}
