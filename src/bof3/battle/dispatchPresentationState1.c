#include "bof3/battle/battle03_internal.h"

/* @behavior dispatches the current local presentation state-1 byte through its
 * table.
 * @source 0x801E31C8
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS dispatchPresentationState1(void) {
  volatile u8* scratch;

  scratch = BATTLE_SCRATCH_CELL_U8PTR;
  D_801EB3B0[scratch[1]]();
}
