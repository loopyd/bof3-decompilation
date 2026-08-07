#include "internal.h"

/* @behavior reports whether one enemy battler's `0xa8` value is large enough for
 * the current average/max threshold pair.
 * @source 0x801DB3E4
 */
u8 func_801DB3E4(u32 arg0, s32 arg1, u32 arg2) {
  u32 value;

  arg1 &= 0xffff;
  arg0 = (arg0 & 0xffu) - 3u;
  value = ((Battle03EnemyHalfRecord*)D_801EB6D8)[arg0].half_00;
  return ((arg1 << 1) <= (s32)value) && ((arg2 & 0xffffu) <= value);
}
