#include "bof3/battle/battle15_internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @returns 1 if the value at offset based on arg0 in D_80145FAA equals 0xE or 0x128.
 * @source 0x800AB4BC
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
s32 func_800AB4BC(s32 arg0) {
  u16 val;

  val = D_80145FAA[(arg0 & 0xFF) * 0xA0];
  if (val != 0xE)
    return val == 0x128;
  return 1;
}
