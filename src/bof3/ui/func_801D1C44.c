#include "bof3/ui/sisyou00_internal.h"

/* @source 0x801D1C44
 * @behavior sums the first count values of the row selected by the first
 *           argument in the shared 0x801CB8DC value table; a count of 100 or
 *           greater returns -1 instead of summing.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_801D1C44(u8 row, u8 count) {
  u8 index;
  s32 total;

  if (count >= 100) {
    return -1;
  }

  total = 0;
  for (index = 0; index < count; index++) {
    total += D_801CB8DC[row][index].value;
  }
  return total;
}
