#include "bof3/bof3.h"

/* @source 0x800C1934
 * @behavior Empty overlay accessor returning the zero result the caller expects; the overlay dispatch table references it as an inert slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 zeroResultBoss037_800C1934(void) {
  return 0;
}
