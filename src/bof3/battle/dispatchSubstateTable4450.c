#include "bof3/battle/battle15_internal.h"

/* @behavior dispatches the secondary-selection substate byte through the function
 * table rooted at `battle_selection_secondary_substate_table`.
 * @source 0x80098388
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable4450(void) {
  D_800B4450[D_801462E4]();
}
