#include "bof3/battle/battle03_internal.h"

/* @behavior reports whether one local battler's `0x98` value is large enough for
 * the current average/max threshold pair.
 * @source 0x801DB3A0
 * @status partial
 * @match 40.00
 * @residual non-exact live audit: 8/17 instructions; 68 original bytes versus 80 current.
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
