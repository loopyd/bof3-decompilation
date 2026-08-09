#include "bof3/battle/battle15_internal.h"

/* @behavior dispatches the slot-selection substate byte through the function table
 * rooted at `battle_selection_slot_substate_table`.
 * @source 0x80096AE8
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void dispatchSubstateTable43c0(void) {
  D_800B43C0[D_801462E4]();
}
