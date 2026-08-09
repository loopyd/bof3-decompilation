#include "bof3/battle/battle15_internal.h"

/* @behavior dispatches the result-selection substate byte through the function
 * table rooted at `battle_selection_result_substate_table`.
 * @source 0x8009773C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable43f4(void) {
  D_800B43F4[D_801462E4]();
}
