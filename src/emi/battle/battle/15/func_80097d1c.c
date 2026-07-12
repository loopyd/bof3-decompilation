#include "internal.h"

/* @behavior dispatches the finalize-selection substate byte through the function
 * table rooted at `battle_selection_finalize_substate_table`.
 * @source 0x80097d1c FUN_80097d1c
 */
void func_80097d1c(void) {
  volatile u8*                           substate_base;
  u32                                    substate;
  BattleSelectionHandler const volatile* table;

  substate_base = (volatile u8*)0x80140000u;
  substate = substate_base[0x62e4];
  table = (BattleSelectionHandler const volatile*)0x800b0000u;
  table[substate + 0x1102]();
}
