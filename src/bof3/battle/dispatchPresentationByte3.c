#include "bof3/battle/battle03_internal.h"

/* @behavior dispatches the current local byte-3 presentation state through its
 * table.
 * @source 0x801E4490
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS dispatchPresentationByte3(void) {
  volatile u8* scratch;

  scratch = BATTLE_SCRATCH_CELL_U8PTR;
  D_801EB430[scratch[3]]();
}
