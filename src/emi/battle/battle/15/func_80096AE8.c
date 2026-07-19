#include "internal.h"

/* @behavior dispatches the slot-selection substate byte through the function table
 * rooted at `battle_selection_slot_substate_table`.
 * @source 0x80096AE8
 */
void func_80096AE8(void) {
  volatile u8*                           substate_base;
  u32                                    substate;
  BattleSelectionHandler const volatile* table;

  substate_base = BATTLE_GAME_RAM_BASE;
  substate = substate_base[0x62e4];
  table = BATTLE_SELECTION_TABLE_BASE;
  table[substate + 0x10f0]();
}
