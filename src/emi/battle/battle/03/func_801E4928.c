#include "internal.h"

/* @behavior dispatches the current queued-result substate byte through its table.
 * @source 0x801E4928
 */
void NO_SIBLING_CALLS func_801E4928(void) {
  volatile u8* scratch;

  scratch = BATTLE_SCRATCH_CELL_U8PTR;
  D_801EB454[scratch[3]]();
}
