#include "bof3/battle/battle03_internal.h"

/* @behavior dispatches the current default-class local byte through its table.
 * @source 0x801E1E7C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS dispatchDefaultClassTable(void) {
  D_801EB27C[D_1F800044->unk_02]();
}
