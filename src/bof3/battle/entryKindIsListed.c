#include "bof3/battle/battle15_internal.h"

/* @behavior Returns whether the selected entry kind is 0x4C, 0x4D, 0xB4, or 0xB5. */

/* @source 0x800AB470
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
s32 entryKindIsListed(u8 arg0) {
  u16 value;

  value = D_80145FAA[arg0 * 160];
  if (value == 0x4C)
    return 1;
  if (value == 0x4D)
    return 1;
  if (value != 0xB4)
    return value == 0xB5;
  return 1;
}
