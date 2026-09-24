#include "bof3/bof3.h"

/* @source 0x800C1A4C
 * @behavior Empty overlay accessor returning the zero result the caller expects; the overlay dispatch table references it as an inert slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 zeroResultBossBoss05116_800C1A4C(void) {
  return 0;
}
