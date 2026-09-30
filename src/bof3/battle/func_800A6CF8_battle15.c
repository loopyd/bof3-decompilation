#include "bof3/battle/battle15_internal.h"

/* @source 0x800A6CF8
 * @behavior Reads the 3-byte row selected by the two index bytes from the
 *   lookup table at 0x800B4D58 and reports whether that entry is the 0xFF
 *   sentinel or appears in the active byte list at 0x801463C4.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 func_800A6CF8(u8 index, u8 value)
{
  u8 count;
  u8 entry;
  u8 i;

  entry = D_800B4D58[index][value];
  if (entry == 0xFF) {
    return 1;
  }
  count = D_801463C7;
  i = 0;
  if (count != 0) {
    do {
      if (D_801463C4[i] == entry) {
        return 1;
      }
      i++;
    } while (i < count);
  }
  return 0;
}
