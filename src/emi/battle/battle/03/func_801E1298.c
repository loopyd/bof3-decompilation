#include "internal.h"

/* @behavior dispatches the current local state-4 byte through its table.
 * @source 0x801E1298
 */
void NO_SIBLING_CALLS func_801E1298(void) {
  volatile u8* scratch;

  scratch = BATTLE_SCRATCH_CELL_U8PTR;
  (*BATTLE_DISPATCH_STATE4(scratch[4]))();
}
