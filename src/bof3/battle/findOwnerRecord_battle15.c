#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800AC754
 * @behavior Scans the thirty 0x98-byte work records and returns the base of the
 * first record whose kind byte is 7 and whose owner byte at +0x8C equals id, or
 * NULL when no record matches.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Matching note: the records are read through the reviewed D_8014688E view
 * (kind at +0x00), while the returned pointer is the 0x98-byte record base six
 * bytes earlier; the original materializes that base once outside the loop and
 * the subtraction folds into the symbol addend.
 */
u8 *findOwnerRecord(u8 id)
{
  u8 i;

  i = 0;
  while (i < 30) {
    if (D_8014688E[i].kind == 7 && D_8014688E[i].owner == id) {
      return (u8 *)&D_8014688E[i] - 0x6;
    }
    i++;
  }
  return NULL;
}
