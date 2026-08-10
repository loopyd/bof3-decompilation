#include "bof3/battle/battle15_internal.h"

/* @behavior Tests whether all ten status slots for a battle entry are nonzero.
 * @source 0x800AA8A4
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
s32 func_800AA8A4(u8 entry)
{
  u8 slot;

  slot = 0;
  while (slot < 10) {
    if (D_80145F7E[entry].slots[slot] == 0) {
      return 0;
    }
    slot++;
  }
  return 1;
}
