#include "internal.h"

/* @behavior dispatches the current local state-2 followup byte through its table.
 * @source 0x801E1CD8
 */
void NO_SIBLING_CALLS dispatchState2FollowupTable(void) {
  volatile u8* scratch;

  scratch = BATTLE_SCRATCH_CELL_U8PTR;
  D_801EB274[scratch[2]]();
}
