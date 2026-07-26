#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @returns 1 if the value at offset based on arg0 in D_80145FAA equals 0xE or 0x128.
 * @source 0x800AB4BC
 */
s32 func_800AB4BC(s32 arg0) {
  u16 val;

  val = (*(volatile u16*)((u32)D_80145FAA + ((arg0 & 0xFF) * 0x140)));
  if (val == 0xE)
    return 1;
  return val == 0x128;
}
