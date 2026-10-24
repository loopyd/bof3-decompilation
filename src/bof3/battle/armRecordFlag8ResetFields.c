#include "bof3/battle/battle15_internal.h"

/* @source 0x8009F9E0
 * @status exact
 * @match 100.00
 * @residual none
 */
/* @behavior Sets the state flag at +0x08 and resets the signed halfwords at +0x04/+0x06. */
void armRecordFlag8ResetFields(void) {
  s16 *ptr;

  ((u8 *)D_8014639C.record)[8] |= 2;
  ((s16 *)D_8014639C.record)[2] = -5;
  ptr = D_8014639C.record;
  ptr[3] = -1;
}
