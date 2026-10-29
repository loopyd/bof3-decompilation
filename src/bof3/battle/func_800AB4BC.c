#include "bof3/battle/battle15_internal.h"

/* @source 0x800AB4BC
 * @behavior Returns 1 when the unsigned halfword at 0x80145FAA + ((u8)arg0)*0x140
 * is 0x000E or 0x0128, otherwise 0.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_800AB4BC(s32 arg0) {
  u16 val;

  val = D_80145FAA[(arg0 & 0xFF) * 0xA0];
  if (val != 0xE)
    return val == 0x128;
  return 1;
}
