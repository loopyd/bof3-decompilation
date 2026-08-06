#include "internal.h"

/* @behavior dispatches the confirm-selection substate byte through the function
 * table rooted at `battle_selection_confirm_substate_table`.
 * @source 0x80096F3C
 */
void dispatchSubstateTable43ec(void) {
  D_800B43EC[D_801462E4]();
}
