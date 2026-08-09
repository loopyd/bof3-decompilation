#include "bof3/battle/battle03_internal.h"

/* @behavior dispatches the current local state-4 byte through its table.
 * @source 0x801E1298
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS dispatchLocalState4Table(void) {
  D_801EB210[D_1F800044->unk_04]();
}
