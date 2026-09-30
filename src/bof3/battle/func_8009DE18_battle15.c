#include "bof3/battle/battle15_internal.h"

/* @source 0x8009DE18
 * @behavior Stores the value returned by func_800A30E8 for the bytes
 *   D_80146374 and D_80146394 into field +0x04 of the selection record
 *   pointer D_801463A0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8009DE18(void) {
  D_801463A0[2] = func_800A30E8(D_80146374, D_80146394);
}
