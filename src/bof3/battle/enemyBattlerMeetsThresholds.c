#include "bof3/battle/battle03_internal.h"

/* @behavior reports whether one enemy battler's `0xa8` value is large enough for
 * the current average/max threshold pair.
 * @source 0x801DB3E4
 * @status partial
 * @match unavailable
 * @residual requeued after forbidden matching aid removal; clean-C byte match and independent review required
 */
u8 enemyBattlerMeetsThresholds(u32 arg0, s32 arg1, u32 arg2) {
  u32 value;

  u32 offset;

  arg1 &= 0xffff;
  do {
    arg0 = (arg0 & 0xffu) - 3u;
    offset = arg0 << 3;
    offset += arg0;
    offset <<= 2;
    offset -= arg0;
    offset <<= 3;
    value = ((Battle03EnemyHalfRecord*)((u8*)D_801EB6D8 + offset))->half_00;
  } while (0);
  return ((arg1 << 1) <= (s32)value) && ((arg2 & 0xffffu) <= value);
}
