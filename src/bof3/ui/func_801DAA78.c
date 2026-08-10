#include "bof3/ui/shop00_internal.h"

/* @source 0x801DAA78
 * @behavior sums the first count values for a shop table row; rejects counts
 *           of 100 or greater.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
s32 func_801DAA78(u8 row, u8 count) {
  u8 i;
  s32 total;

  if (count >= 100) {
    return -1;
  }

  total = 0;
  for (i = 0; i < count; i++) {
    total += D_801CB8DC[row][i].value;
  }
  return total;
}
