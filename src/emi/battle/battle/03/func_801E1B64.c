#include "internal.h"

/* @behavior dispatches the current local state-2 event byte through its table.
 * @source 0x801E1B64
 */
void NO_SIBLING_CALLS func_801E1B64(void) {
  volatile u8* scratch;

  scratch = BATTLE_SCRATCH_CELL_U8PTR;
  (*BATTLE_DISPATCH_STATE2_EVENT(scratch[2]))();
}
