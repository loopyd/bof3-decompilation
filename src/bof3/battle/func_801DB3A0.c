#include "bof3/battle/battle03_internal.h"

/* @behavior reports whether one local battler's `0x98` value is large enough for
 * the current average/max threshold pair.
 * @source 0x801DB3A0
 * @status partial
 * @match 61.11
 * @residual 11/18 instructions; 68 original bytes versus 72 current. Indexed
 * address allocation, comparison signedness and return scheduling still differ.
 */
u8 func_801DB3A0(u32 arg0, u32 arg1, u32 arg2) {
  u16 value;

  arg1 &= 0xffffu;
  arg0 &= 0xffu;
  value = ((Battle03LocalHalfRecord*)D_80145F28)[arg0].half_00;
  if ((arg1 << 1) > value) {
    return 0;
  }
  return value >= (arg2 & 0xffffu);
}
