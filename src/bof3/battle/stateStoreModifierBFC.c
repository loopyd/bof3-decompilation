#include "bof3/battle/battle15_internal.h"

/* @source 0x800A0BF4
 * @behavior Stores the value returned by mathNegatedScaleRoll for the bytes
 *   D_80146374 and D_80146394 into field +0x04 of the selection record
 *   pointer D_8014639C.record, then applies the 0xBFC flags value to the byte
 *   D_80146394 through func_800A36F0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateStoreModifierBFC(void) {
  *(volatile s16 *)&D_8014639C.record[2] = mathNegatedScaleRoll(D_80146374, D_80146394);
  func_800A36F0(D_80146394, 0xBFC);
}
