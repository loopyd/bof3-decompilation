#include "internal.h"

/* @source 0x801DFBDC
 * @behavior dispatches byte 2 of the scratchpad-resident state object through
 * its handler table.
 */
void NO_SIBLING_CALLS dispatchScratchByte2Table(void) {
  u8 state_index;

  state_index = BATTLE_SCRATCH_CELL_U8PTR[2];
  D_801EB1BC[state_index]();
}
