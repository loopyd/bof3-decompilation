#include "bof3/battle/battle15_internal.h"

/* @source 0x800AB470
 * @behavior Returns 1 when the unsigned halfword at 0x80145FAA + 0x140 * (u8)arg0 is
 * 0x4C, 0x4D, 0xB4 or 0xB5; returns 0 otherwise.
 * @status exact
 * @match 100.00
 * @residual none
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
