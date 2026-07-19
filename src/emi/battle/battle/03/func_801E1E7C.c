#include "internal.h"

/* @behavior dispatches the current default-class local byte through its table.
 * @source 0x801E1E7C
 */
void NO_SIBLING_CALLS func_801E1E7C(void) {
  Battle03Handler handler;
  u8              index;

  index = BATTLE_SCRATCH_CELL_WORKPTR->unk_02;
  handler = *BATTLE_DISPATCH_DEFAULT_CLASS(index);
  handler();
}
