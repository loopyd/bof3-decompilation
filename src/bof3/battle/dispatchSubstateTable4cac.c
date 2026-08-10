#include "bof3/battle/battle15_internal.h"

/* @source 0x800A4688
 * @behavior dispatches the battle selection handler indexed by D_801462E3.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable4cac(void) {
  battleSelectionHandlerTable4CAC[D_801462E3]();
}
