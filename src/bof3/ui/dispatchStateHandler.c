#include "bof3/ui/game00_internal.h"

/* @behavior dispatches through the indexed handler table at stateHandlerTable
 * using the s8 state selector at D_801448EA.
 * @source 0x801A7BF0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchStateHandler(void) {
  stateHandlerTable[(s32)D_801448EA]();
}
