#include "bof3/battle/battle15_internal.h"

/* @source 0x800A4C08
 * @behavior dispatches the byte-selected battle handler through the local table.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable4ce4(void) {
  D_800B4CE4[D_801462E4]();
}
