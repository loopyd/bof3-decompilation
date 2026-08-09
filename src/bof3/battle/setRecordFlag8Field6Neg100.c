#include "bof3/battle/battle15_internal.h"

/* @behavior initializes two adjacent battle-state fields through D_801463A0.
 * @source 0x8009E224
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void setRecordFlag8Field6Neg100(void) {
  *((volatile u8*)D_801463A0 + 8) = 2;
  (*(s16*)((u32)D_801463A0 + 6)) = -0x64;
}
