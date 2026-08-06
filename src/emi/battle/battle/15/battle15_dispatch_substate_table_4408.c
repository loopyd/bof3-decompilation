#include "internal.h"

/* @behavior dispatches the finalize-selection substate byte through the function
 * table rooted at `battle_selection_finalize_substate_table`.
 * @source 0x80097D1C
 */
void battle15_dispatch_substate_table_4408(void) {
  D_800B4408[D_801462E4]();
}
