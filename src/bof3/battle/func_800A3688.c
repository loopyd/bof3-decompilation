#include "bof3/battle/battle15_internal.h"

/* @source 0x800A3688
 * @behavior Returns 1 when the low byte of arg0 is at least 3. Otherwise, returns 1
 * if byte +0x85 through the pointer loaded from 0x80146250 differs from the low
 * byte of arg1, or 0 if they are equal.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 func_800A3688(s32 arg0, s32 arg1) {
  volatile u8* entry;

  if ((u32)(arg0 & 0xFF) < 3U) {
    entry = (volatile u8*)D_80146250;
    return (*(entry + 0x85)) != (arg1 & 0xFF);
  }
  return 1;
}
