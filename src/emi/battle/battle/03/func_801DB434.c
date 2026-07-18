#include "internal.h"

/* @behavior scans one six-entry threshold row and returns the first index whose
 * value exceeds the input byte, or `6` if none match.
 * @source 0x801DB434
 */
u32 func_801DB434(u8 arg0, u32 arg1) {
  const volatile u8* row;
  u8                 index;

  row = (const volatile u8*)((const u8*)0x801f0000u + (s16)0xaf88u);
  row += (arg1 & 0xffu) * 6u;
  index = 0u;
  while (index < 6u) {
    if (arg0 < row[index]) {
      return index;
    }
    index += 1u;
  }
  return 6u;
}
