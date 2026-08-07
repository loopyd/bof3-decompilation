#include "internal.h"

#include "base/barrier.h"

/* @behavior dispatches byte 2 of the scratchpad-resident state object through
 * its handler table.
 * @source 0x801D0E9C
 */
void NO_SIBLING_CALLS func_801D0E9C(void) {
  ScenarioSce10effScratch* scratch;
  u8                       state_index;

  scratch = SPAD_PTR_SLOT(ScenarioSce10effScratch, 0x44u);
  state_index = ((u8*)scratch)[2];
  D_801D2708[state_index]();
}
