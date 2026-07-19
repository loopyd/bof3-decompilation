#include "internal.h"

/* @behavior reports whether one local battler's `0x98` value is large enough for
 * the current average/max threshold pair.
 * @source 0x801DB3A0
 */
u8 func_801DB3A0(u32 arg0, u32 arg1, u32 arg2) {
  u16 value;

  arg1 &= 0xffffu;
  arg0 &= 0xffu;
  value = BATTLE_LOCAL_ABS_HALF_5F28(arg0);
  if ((arg1 << 1) > value) {
    return 0;
  }
  return value >= (arg2 & 0xffffu);
}
