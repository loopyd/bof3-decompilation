#include "internal.h"

/* @behavior dispatches the confirm-selection substate byte through the function
 * table rooted at `battle_selection_confirm_substate_table`.
 * @source 0x80096f3c FUN_80096f3c
 */
void NO_SIBLING_CALLS func_80096f3c(void) {
  volatile u8*                           substate_base;
  u32                                    substate;
  BattleSelectionHandler const volatile* table;

  substate_base = (volatile u8*)0x80140000u;
  substate = substate_base[0x62e4];
  table = (BattleSelectionHandler const volatile*)0x800b0000u;
  table[substate + 0x10fb]();
}
