#include "bof3/bof3.h"

/* @source 0x800C21BC
 * @behavior Empty overlay accessor returning the zero result the caller expects; the overlay dispatch table references it as an inert slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 zeroResultBossBoss00816_800C21BC(void) {
  return 0;
}
