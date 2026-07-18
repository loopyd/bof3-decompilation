#include "internal.h"

/* @behavior evaluates whether one battler's countdown/retry state should trigger on
 * this frame using both the short counter table and the long retry table.
 * @source 0x801DDCB4
 */
u8 func_801DDCB4(u32 arg0) {
  u8 counter;
  u8 retry_index;

  if (arg0 < 3u) {
    counter = BATTLE_LOCAL_BYTE_121(&BATTLE_LOCAL_WORK_ARRAY[arg0]);
  } else {
    counter =
        BATTLE_ENEMY_BYTE_FD(&BATTLE_ENEMY_WORK_ARRAY[(arg0 - 3u) & 0xffu]);
  }
  if (counter == 0u) {
    return 0u;
  }

  if (counter < 3u) {
    if ((func_8017E3D4() % 100) <= BATTLE_COUNTER_TABLE_AFFC[counter]) {
      return 1u;
    }
  } else {
    if ((func_8017E3D4() % 100) < 0x4cu) {
      return 1u;
    }
  }

  if (arg0 < 3u) {
    retry_index = BATTLE_LOCAL_BYTE_A6(&BATTLE_LOCAL_WORK_ARRAY[arg0]);
  } else {
    retry_index =
        BATTLE_ENEMY_BYTE_E6(&BATTLE_ENEMY_WORK_ARRAY[(arg0 - 3u) & 0xffu]);
  }

  return (func_8017E3D4() % 10000) <=
         (10000 - ((s32)BATTLE_RETRY_TABLE_AFF4[retry_index] * 50));
}
