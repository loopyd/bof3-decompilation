#include "bof3/battle/battle03_internal.h"

/* @behavior dispatches the current local state-2 class byte through its table.
 * @source 0x801E1670
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS dispatchState2ClassTable(void) {
  volatile u8* scratch;

  scratch = BATTLE_SCRATCH_CELL_U8PTR;
  D_801EB224[scratch[2]]();
}
