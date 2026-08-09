#include "bof3/battle/battle03_internal.h"

/* @behavior dispatches the current local substate-3 byte through its table.
 * @source 0x801E046C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS dispatchLocalSubstate3Table(void) {
  D_801EB1E0[BATTLE_LOCAL_SCRATCH_PTR->unk_03]();
}
