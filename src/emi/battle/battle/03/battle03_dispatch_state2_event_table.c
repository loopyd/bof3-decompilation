#include "internal.h"

/* @behavior dispatches the current local state-2 event byte through its table.
 * @source 0x801E1B64
 */
void NO_SIBLING_CALLS battle03_dispatch_state2_event_table(void) {
  D_801EB26C[BATTLE_SCRATCH_CELL_U8PTR[2]]();
}
