#include "internal.h"

/* @behavior dispatches the secondary-selection substate byte through the function
 * table rooted at `battle_selection_secondary_substate_table`.
 * @source 0x80098388
 */
void NO_SIBLING_CALLS func_80098388(void) {
  volatile u8*                           battle_selection_state;
  BattleSelectionHandler const volatile* battle_selection_substate_table;

  battle_selection_state = (volatile u8*)0x80140000u;
  battle_selection_substate_table =
      (BattleSelectionHandler const volatile*)0x800b0000u;
  battle_selection_substate_table[battle_selection_state[0x62e4u] + 0x1114u]();
}
