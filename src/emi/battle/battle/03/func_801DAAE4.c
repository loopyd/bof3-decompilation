#include "internal.h"

extern int rand(void);
typedef struct Battle03RankEntry {
  s16 value;
  u16 index;
} Battle03RankEntry;

/* @behavior builds and sorts the mixed local/enemy ranking list used by the owner
 * selection bytes at `0x8014630c`, then resets the current owner cursor.
 * @source 0x801DAAE4
 */
void func_801DAAE4(void) {
  Battle03RankEntry entries[11];
  u16               max_local_value;
  u8                count;
  u8                index;

  max_local_value = 0u;
  index = 0u;
  while (index < 3u) {
    volatile Battle03LocalWork* battle_work;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[index];
    if ((BATTLE_GLOBAL_BYTE_63BA == 0u) ||
        ((BATTLE_LOCAL_WORD_128(battle_work) & 0x8000u) != 0u)) {
      if ((func_801DB9E4(index) != 0u) &&
          (max_local_value < BATTLE_LOCAL_HALF_98(battle_work))) {
        max_local_value = BATTLE_LOCAL_HALF_98(battle_work);
      }
    }
    index += 1u;
  }

  count = 0u;
  index = 0u;
  while (index < 3u) {
    volatile Battle03LocalWork* battle_work;
    s16                         score;

    battle_work = &BATTLE_LOCAL_WORK_ARRAY[index];
    if ((BATTLE_GLOBAL_BYTE_63BA == 0u) ||
        ((BATTLE_LOCAL_WORD_128(battle_work) & 0x8000u) != 0u)) {
      if (func_801DB9E4(index) != 0u) {
        score = 0;
        if (BATTLE_LOCAL_BYTE_119(battle_work) == 4u) {
          u8 kind_mode;

          kind_mode = *(
              volatile u8*)(0x801ca719u +
                            ((u32)BATTLE_LOCAL_HALF_11A(battle_work) * 0x14u));
          if (kind_mode == 1u || kind_mode == 3u) {
            score = 2;
          } else if (kind_mode == 0u) {
            score = 4;
          }
        }
        if (BATTLE_LOCAL_BYTE_119(battle_work) == 5u) {
          score = 1;
        }
        score = (s16)((score * BATTLE_PERCENT_TABLE_AF3C[func_801DB434(
                                   BATTLE_LOCAL_BYTE_7A(battle_work), 1u)]) /
                      100);
        entries[count].value = (s16)(score + BATTLE_LOCAL_HALF_98(battle_work));
        entries[count].index = index;
        count += 1u;
      }
    }
    index += 1u;
  }

  index = 3u;
  while (index < 0x0bu) {
    volatile Battle03EnemyWork* battle_work;

    battle_work = &BATTLE_ENEMY_WORK_ARRAY[index - 3u];
    if ((BATTLE_GLOBAL_BYTE_63BA == 0u) ||
        ((BATTLE_ENEMY_WORD_104(battle_work) & 0x8000u) != 0u)) {
      if (func_801DB9E4(index) != 0u) {
        u32 rank;
        s8  random_bonus;

        rank = func_801DB434(BATTLE_ENEMY_BYTE_88(battle_work), 0u);
        random_bonus =
            BATTLE_RANDOM_BONUS_TABLE_AF48[(rank * 0x10u) + (rand() & 0xfu)];
        entries[count].value =
            (s16)(BATTLE_ENEMY_HALF_A8(battle_work) + random_bonus);
        entries[count].index = index;
        count += 1u;
      }
    }
    index += 1u;
  }

  BATTLE_GLOBAL_BYTE_6323 = count;
  if (count > 1u) {
    u8 left;

    left = 0u;
    while (left + 1u < count) {
      u8 right;

      right = left + 1u;
      while (right < count) {
        if (entries[left].value < entries[right].value) {
          Battle03RankEntry temp;

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
  while (index < 0x0bu) {
    BATTLE_GLOBAL_BYTE_630C(index) = 0xffu;
    index += 1u;
  }

  index = 0u;
  while (index < BATTLE_GLOBAL_BYTE_6323) {
    BATTLE_GLOBAL_BYTE_630C(index) = (u8)entries[index].index;
    index += 1u;
  }

  BATTLE_GLOBAL_BYTE_6322 = 0u;
}
