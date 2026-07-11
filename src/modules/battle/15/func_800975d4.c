#include "internal.h"

/* @behavior once the live grid gate clears, restores the battle owner tuple to the
 * shared selection root and reapplies the local panel-task band reset helper.
 * @source 0x800975d4 FUN_800975d4
 */
void NO_SIBLING_CALLS func_800975d4(void) {
  volatile u8* battle_selection_state;
  volatile u8* battle_selection_lock;

  battle_selection_state = (volatile u8*)0x80140000u;
  battle_selection_lock = (volatile u8*)0x80150000u;
  if (battle_selection_lock[-0x7a8d] != 0u) {
    return;
  }

  battle_selection_state[0x62e1u] = 1u;
  battle_selection_state[0x62e2u] = 0u;
  battle_selection_state[0x62e3u] = 0u;
  func_8009b20c();
}
