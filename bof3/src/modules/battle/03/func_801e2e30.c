#include "internal.h"

/* does: chooses one enabled slot by weighted random selection, using the two-
 * or three-entry weight table selected by the global mode byte.
 * @source: 0x801e2e30 FUN_801e2e30
 */
u8 func_801e2e30(void) {
  u8  total;
  u8  sum;
  u8  index;
  u8  count;
  s32 random_value;

  total = 0u;
  sum = 0u;
  if (BOF3_BATTLE_GLOBAL_BYTE_62F0 == 2u) {
    index = 0u;
    do {
      if (func_801db524(index) == 0u) {
        total += BOF3_BATTLE_WEIGHT_TABLE_0394
            [index + ((u32)BOF3_BATTLE_GLOBAL_BYTE_44F58 * 2u)];
      }
      index += 1u;
    } while (index < 2u);

    random_value = func_8017e3d4();
    index = 0u;
    do {
      if (func_801db524(index) == 0u) {
        sum += BOF3_BATTLE_WEIGHT_TABLE_0394
            [index + ((u32)BOF3_BATTLE_GLOBAL_BYTE_44F58 * 2u)];
        if ((u8)(random_value % total) < sum) {
          return index;
        }
      }
      index += 1u;
    } while (index < 2u);

    return index;
  }

  if (BOF3_BATTLE_GLOBAL_BYTE_62F0 != 3u) {
    return 0u;
  }

  index = 0u;
  do {
    if (func_801db524(index) == 0u) {
      total += BOF3_BATTLE_WEIGHT_TABLE_039C
          [index + ((u32)BOF3_BATTLE_GLOBAL_BYTE_44F58 * 3u)];
    }
    index += 1u;
  } while (index < 3u);

  random_value = func_8017e3d4();
  count = 0u;
  do {
    index = count;
    if (func_801db524(index) == 0u) {
      sum += BOF3_BATTLE_WEIGHT_TABLE_039C[index +
                                           ((u32)BOF3_BATTLE_GLOBAL_BYTE_44F58 *
                                            3u)];
      if ((u8)(random_value % total) < sum) {
        return index;
      }
    }
    count += 1u;
  } while (count < 3u);

  return count;
}
