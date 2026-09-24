#include "bof3/bof3.h"

/* @source 0x800C1DA8
 * @behavior Empty overlay accessor returning the zero result the caller expects; the overlay dispatch table references it as an inert slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 zeroResultBossBoss03316_800C1DA8(void) {
  return 0;
}
