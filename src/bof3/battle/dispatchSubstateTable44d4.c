#include "bof3/battle/battle15_internal.h"

/* @source 0x8009B44C
 * @behavior dispatches the battle selection handler indexed by D_801462E4.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable44d4(void) {
  D_800B44D4[D_801462E4]();
}
