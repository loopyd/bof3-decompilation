#include "bof3/battle/battle15_internal.h"

/* @source 0x800A0BF4
 * @behavior Stores the value returned by func_800A30E8 for the bytes
 *   D_80146374 and D_80146394 into field +0x04 of the selection record
 *   pointer D_801463A0, then applies the 0xBFC flags value to the byte
 *   D_80146394 through func_800A36F0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_800A0BF4(void) {
  *(volatile s16 *)&D_801463A0[2] = func_800A30E8(D_80146374, D_80146394);
  func_800A36F0(D_80146394, 0xBFC);
}
