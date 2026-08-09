#include "bof3/battle/battle15_internal.h"

/* @behavior dispatches the current battle selection state through the table at D_800B4D30.
 * @source 0x800A5FF0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable4d30(void) {
  D_800B4D30[D_801462E4]();
}
