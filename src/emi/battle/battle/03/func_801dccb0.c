#include "internal.h"

/* @behavior averages the current local battler `0x96` values and blends the result
 * with the queued halfword at `0x801ec2ee`.
 * @source 0x801dccb0 FUN_801dccb0
 */
u32 func_801dccb0(void) {
  s32 total;
  u32 count;
  u8  index;

  total = 0;
  index = 0u;
  count = *(volatile u8*)(0x80140000u + 0x62f0u);
  if (count != 0u) {
    do {
      total += *(volatile u16*)(0x80140000u + ((u32)index * 0x140u) + 0x5f26u);
      index += 1u;
    } while ((u32)index < count);
  }

  return ((((u16)total / *(volatile u8*)(0x80140000u + 0x62f0u))) +
          *(volatile u16*)(0x801f0000u - 0x3d12u)) >>
         1;
}
