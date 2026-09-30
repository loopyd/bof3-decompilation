#include "bof3/bof3.h"

/* @source 0x801FAB6C
 * @behavior Empty overlay accessor returning the negative-one result the caller expects; the overlay dispatch table references it as an inert slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 negativeOneResultScenarioScena1100_801FAB6C(void) {
  return -1;
}
