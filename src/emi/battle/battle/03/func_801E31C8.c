#include "internal.h"

/* @behavior dispatches the current local presentation state-1 byte through its
 * table.
 * @source 0x801E31C8
 */
void NO_SIBLING_CALLS func_801E31C8(void) {
  volatile u8* scratch;

  scratch = BATTLE_SCRATCH_CELL_U8PTR;
  D_801EB3B0[scratch[1]]();
}
