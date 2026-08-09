#include "bof3/battle/battle03_internal.h"

/* @behavior dispatches the current local state-2 event byte through its table.
 * @source 0x801E1B64
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS dispatchState2EventTable(void) {
  D_801EB26C[BATTLE_SCRATCH_CELL_U8PTR[2]]();
}
