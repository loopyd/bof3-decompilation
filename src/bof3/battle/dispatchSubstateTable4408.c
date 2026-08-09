#include "bof3/battle/battle15_internal.h"

/* @behavior dispatches the finalize-selection substate byte through the function
 * table rooted at `battle_selection_finalize_substate_table`.
 * @source 0x80097D1C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable4408(void) {
  D_800B4408[D_801462E4]();
}
