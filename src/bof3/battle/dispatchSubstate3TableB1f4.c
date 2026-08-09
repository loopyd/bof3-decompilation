#include "bof3/battle/battle03_internal.h"

/* @behavior dispatches the handler selected by the non-volatile scratchpad
 * pointer cell's local substate byte `3` through the 0x801EB1F4 table.
 * @source 0x801E0744
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS dispatchSubstate3TableB1f4(void) {
  D_801EB1F4[((volatile Battle03LocalWork*)battleWork)->unk_03]();
}
