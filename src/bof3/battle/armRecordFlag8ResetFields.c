#include "bof3/battle/battle15_internal.h"

/* @source 0x8009F9E0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
/* @behavior Sets the state flag at +0x08 and resets the signed halfwords at +0x04/+0x06. */
void armRecordFlag8ResetFields(void) {
  s16 *ptr;

  ((volatile u8 *)D_801463A0)[8] |= 2;
  ((volatile s16 *)D_801463A0)[2] = -5;
  ptr = D_801463A0;
  ptr[3] = -1;
}
