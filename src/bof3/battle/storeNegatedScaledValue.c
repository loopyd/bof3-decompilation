#include "bof3/battle/battle15_internal.h"

/* @source 0x8009DE18
 * @behavior Stores the value returned by mathNegatedScaleRoll for the bytes
 *   D_80146374 and D_80146394 into field +0x04 of the selection record
 *   pointer D_8014639C.record.
 * @status exact
 * @match 100.00
 * @residual none
 */
void storeNegatedScaledValue(void) {
  D_8014639C.record[2] = mathNegatedScaleRoll(D_80146374, D_80146394);
}
