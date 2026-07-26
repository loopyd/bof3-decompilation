#include "internal.h"

/* @behavior UNKNOWN: exact behavior is not yet documented. */

/* @returns whether the byte at offset 0x84 of D_80146250 differs from arg1 (masked).
 * Only checks when (arg0 & 0xFF) < 3.
 * @source 0x800A36BC
 */
s32 func_800A36BC(s32 arg0, s32 arg1) {
  volatile u8* entry;

  if ((u32)(arg0 & 0xFF) < 3U) {
    entry = (volatile u8*)D_80146250;
    return (*(entry + 0x84)) != (arg1 & 0xFF);
  }
  return 1;
}
