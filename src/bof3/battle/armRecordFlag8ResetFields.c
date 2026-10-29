#include "bof3/battle/battle15_internal.h"

/* @source 0x8009F9E0
 * @behavior ORs 2 into byte +0x08, then writes -5 and -1 to the halfwords at +0x04 and
 * +0x06, reloading the record pointer at 0x801463A0 for each operation.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armRecordFlag8ResetFields(void) {
  s16 *ptr;

  ((u8 *)D_8014639C.record)[8] |= 2;
  ((s16 *)D_8014639C.record)[2] = -5;
  ptr = D_8014639C.record;
  ptr[3] = -1;
}
