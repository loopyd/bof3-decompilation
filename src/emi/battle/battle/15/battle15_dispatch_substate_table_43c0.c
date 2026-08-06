#include "internal.h"

/* @behavior dispatches the slot-selection substate byte through the function table
 * rooted at `battle_selection_slot_substate_table`.
 * @source 0x80096AE8
 */
void battle15_dispatch_substate_table_43c0(void) {
  D_800B43C0[D_801462E4]();
}
