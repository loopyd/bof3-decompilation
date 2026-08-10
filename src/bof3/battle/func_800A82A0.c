#include "bof3/battle/battle15_internal.h"

/* @source 0x800A82A0
 * @behavior Tests whether an 8-bit value occurs in the active byte list.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
u32 func_800A82A0(u8 value)
{
  u8 i;
  u8 count;

  count = D_801463C7;
  for (i = 0; i < count; i++) {
    if (D_801463C4[i] == value) {
      return 1;
    }
  }
  return 0;
}
