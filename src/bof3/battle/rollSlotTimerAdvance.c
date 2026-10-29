#include "bof3/battle/battle03_internal.h"

/* @source 0x801E6088
 * @behavior Decrements byte +0x09 through D_801EC2E0. When its old value was
 * zero, increments byte +0x01 and resets byte +0x09 to 4; otherwise leaves the
 * decremented byte in place.
 * @status exact
 * @match 100.00
 * @residual none
 */
void rollSlotTimerAdvance(void) {
  if (D_801EC2E0->unk_09-- == 0) {
    D_801EC2E0->unk_01++;
    D_801EC2E0->unk_09 = 4;
  }
}
