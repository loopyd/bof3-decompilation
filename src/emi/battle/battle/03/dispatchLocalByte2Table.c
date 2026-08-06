#include "internal.h"

/* @behavior dispatches the current local byte-2 state through its handler table.
 * @source 0x801DFA1C
 */
void NO_SIBLING_CALLS dispatchLocalByte2Table(void) {
  u8 state_index;

  state_index = BATTLE_SCRATCH_CELL_U8PTR[2];
  D_801EB1B4[state_index]();
}
