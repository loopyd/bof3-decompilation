#include "bof3/battle/battle15_internal.h"

/* @source 0x8009B274
 * @behavior dispatches the battle selection handler indexed by the current state.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable44c8(void) {
  battleSelectionHandlerTable44C8[D_801462E3]();
}
