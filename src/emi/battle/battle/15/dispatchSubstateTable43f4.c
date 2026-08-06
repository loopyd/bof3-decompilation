#include "internal.h"

/* @behavior dispatches the result-selection substate byte through the function
 * table rooted at `battle_selection_result_substate_table`.
 * @source 0x8009773C
 */
void dispatchSubstateTable43f4(void) {
  D_800B43F4[D_801462E4]();
}
