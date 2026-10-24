#include "bof3/battle/battle15_internal.h"

/* @stores 0x-14 to offset 2 of the pointer loaded from D_8014639C.record.
 * @behavior stores signed halfword -20 at byte offset 4 through D_8014639C.record
 * @source 0x8009DE50
 * @status exact
 * @match 100.00
 * @residual none
 */
void setRecordField4Neg20(void) {
  ((s16*)D_8014639C.record)[2] = -20;
}
