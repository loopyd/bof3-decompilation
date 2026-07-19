#include "internal.h"

/* @behavior reports whether one enemy battler's `0xa8` value is large enough for
 * the current average/max threshold pair.
 * @source 0x801DB3E4
 */
u8 func_801DB3E4(u32 arg0, u32 arg1, u32 arg2) {
  u32           index;
  u32           value;
  volatile u16* ptr;

  index = (arg0 & 0xffu) - 3u;
  ptr = BATTLE_ENEMY_TABLE_6D8;
  value = ptr[index * 0x8cu];
  return (((arg1 & 0xffffu) << 1) <= value) && ((arg2 & 0xffffu) <= value);
}
