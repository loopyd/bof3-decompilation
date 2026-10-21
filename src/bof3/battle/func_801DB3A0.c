#include "bof3/battle/battle03_internal.h"

/* @behavior reports whether one local battler's `0x98` value is large enough for
 * the current average/max threshold pair.
 * @source 0x801DB3A0
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 func_801DB3A0(u32 arg0, u32 arg1, u32 arg2) {
  u32 value;

  arg1 &= 0xffffu;
  do {
    arg0 &= 0xffu;
    value = arg0 << 2;
    value += arg0;
    value <<= 6;
    value = ((Battle03LocalHalfRecord*)((u8*)D_80145F28 + value))->half_00;
  } while (0);
  if ((s32)value >= (s32)(arg1 << 1)) {
    return (arg2 & 0xffffu) <= value;
  }
  return 0;
}
