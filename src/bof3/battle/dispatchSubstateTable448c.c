#include "bof3/battle/battle15_internal.h"

/* @behavior dispatches the current battle-selection substate through its handler table.
 * @source 0x80099F90
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable448c(void) {
  D_800B448C[D_801462E4]();
}
