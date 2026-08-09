#include "bof3/battle/battle03_internal.h"

/* @behavior dispatches the current alternate local state-3 byte through its table.
 * @source 0x801E1450
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS dispatchAltState3Table(void) {
  volatile Battle03LocalWork* work;

  work = BATTLE_LOCAL_SCRATCH_PTR;
  D_801EB218[work->unk_03]();
}
