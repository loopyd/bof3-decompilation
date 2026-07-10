#include "internal.h"

/* does: reports whether one enemy battler's `0xa8` value is large enough for
 * the current average/max threshold pair.
 * @source: 0x801db3e4 FUN_801db3e4
 */
u8 func_801db3e4(u32 arg0, u32 arg1, u32 arg2) {
  u32           index;
  u32           value;
  volatile u16* ptr;

  index = (arg0 & 0xffu) - 3u;
  ptr = (volatile u16*)0x801eb6d8u;
  value = ptr[index * 0x8cu];
  return (((arg1 & 0xffffu) << 1) <= value) && ((arg2 & 0xffffu) <= value);
}
