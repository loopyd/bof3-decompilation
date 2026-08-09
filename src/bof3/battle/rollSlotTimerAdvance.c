#include "bof3/battle/battle03_internal.h"

/* @source 0x801E6088
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
/* @behavior Decrements the timer; when its post-decrement value is zero, increments the selector and resets the timer to 4. */
void rollSlotTimerAdvance(void) {
  if (D_801EC2E0->unk_09-- == 0) {
    D_801EC2E0->unk_01++;
    D_801EC2E0->unk_09 = 4;
  }
}
