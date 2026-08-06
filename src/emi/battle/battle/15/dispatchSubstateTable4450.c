#include "internal.h"

/* @behavior dispatches the secondary-selection substate byte through the function
 * table rooted at `battle_selection_secondary_substate_table`.
 * @source 0x80098388
 */
void dispatchSubstateTable4450(void) {
  D_800B4450[D_801462E4]();
}
