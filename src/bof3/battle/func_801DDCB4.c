#include "bof3/battle/battle03_internal.h"

extern int rand(void);
/* @behavior evaluates whether one battler's countdown/retry state should trigger on
 * this frame using both the short counter table and the long retry table.
 * @source 0x801DDCB4
 * @status partial
 * @match 37.40
 * @residual non-exact live audit: 49/114 instructions; 456 original bytes versus 524 current.
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
    if ((rand() % 100) <= BATTLE_COUNTER_TABLE_AFFC[counter]) {
      return 1u;
    }
  } else {
    if ((rand() % 100) < 0x4cu) {
      return 1u;
    }
  }

  if (arg0 < 3u) {
    retry_index = BATTLE_LOCAL_BYTE_A6(&BATTLE_LOCAL_WORK_ARRAY[arg0]);
  } else {
    retry_index =
        BATTLE_ENEMY_BYTE_E6(&BATTLE_ENEMY_WORK_ARRAY[(arg0 - 3u) & 0xffu]);
  }

  return (rand() % 10000) <=
         (10000 - ((s32)BATTLE_RETRY_TABLE_AFF4[retry_index] * 50));
}
