#include "bof3/battle/battle15_internal.h"

/* @behavior computes a negated, table-scaled battle value for a selector.
 * @source 0x800A30E8
 */
s32 func_800A30E8(u32 unused, u8 selector)
{
  u32 amount = D_801EC312 + 100;
  u32 record = D_801463C0;
  u32 base;
  u32 index;
  s32 scale;

  (void)unused;
  record = record * 20;
  amount = amount * 100;
  base = D_801CA71B[record];
  base = (base * amount) / 100;
  index = selector;
  if (index < 3) {
    index = D_80145F34[index * 320];
  } else {
    index = D_801EB6E4[(index - 3) * 280];
  }
  scale = ((s32)base * D_800B497C[index]) / 10000;
  return (s16)-scale;
}
