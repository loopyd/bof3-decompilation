#include "bof3/bof3.h"

/* @source 0x800C2378
 * @behavior Empty overlay accessor returning the zero result the caller expects; the overlay dispatch table references it as an inert slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 zeroResultBossBoss03416_800C2378(void) {
  return 0;
}
