#include "internal.h"

/* @behavior dispatches the current local byte-3 presentation state through its
 * table.
 * @source 0x801E4490
 */
void NO_SIBLING_CALLS func_801E4490(void) {
  volatile u8* scratch;

  scratch = BATTLE_SCRATCH_CELL_U8PTR;
  (*BATTLE_DISPATCH_PRESENTATION_BYTE3(scratch[3]))();
}
