#include "bof3/battle/battle15_internal.h"

/* @behavior Returns 1 when the low byte of arg0 is at least 3. Otherwise compares
 * the byte at offset 0x84 of the pointer loaded from D_80146250 with the low
 * byte of arg1, returning 1 when they differ and 0 when they are equal.
 * @source 0x800A36BC
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_800A36BC(s32 arg0, s32 arg1) {
  volatile u8* entry;

  if ((u32)(arg0 & 0xFF) < 3U) {
    entry = (volatile u8*)D_80146250;
    return (*(entry + 0x84)) != (arg1 & 0xFF);
  }
  return 1;
}
