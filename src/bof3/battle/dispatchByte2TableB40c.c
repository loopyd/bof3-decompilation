#include "bof3/battle/battle03_internal.h"

/* @source 0x801E3B68
 * @behavior dispatches through D_801EB40C using byte two of the non-volatile
 * scratchpad pointer cell at 0x1F800044.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS dispatchByte2TableB40c(void) {
  D_801EB40C[((Battle03LocalWork*)battleWork)->unk_02]();
}
