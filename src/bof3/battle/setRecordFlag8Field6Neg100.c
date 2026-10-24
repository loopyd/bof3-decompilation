#include "bof3/battle/battle15_internal.h"

/* @behavior initializes two adjacent battle-state fields through D_8014639C.record.
 * @source 0x8009E224
 * @status exact
 * @match 100.00
 * @residual none
 */
void setRecordFlag8Field6Neg100(void) {
  *((volatile u8*)D_8014639C.record + 8) = 2;
  (*(s16*)((u32)D_8014639C.record + 6)) = -0x64;
}
