#include "internal.h"

/* @behavior dispatches the confirm-selection substate byte through the function
 * table rooted at `battle_selection_confirm_substate_table`.
 * @source 0x80096F3C
 */
void NO_SIBLING_CALLS func_80096F3C(void) {
  volatile u8*                           substate_base;
  u32                                    substate;
  BattleSelectionHandler const volatile* table;

  substate_base = BATTLE_GAME_RAM_BASE;
  substate = substate_base[0x62e4];
  table = BATTLE_SELECTION_TABLE_BASE;
  table[substate + 0x10fb]();
}
