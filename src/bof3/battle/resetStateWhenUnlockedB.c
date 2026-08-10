#include "bof3/battle/battle15_internal.h"

/* @source 0x800989B4
 * @behavior when the byte at 0x80148573 is clear, writes 1, 0, 0 to
 * 0x801462E1..0x801462E3 and calls runPanelTasks16To19.
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void NO_SIBLING_CALLS resetStateWhenUnlockedB(void) {
  volatile u8* battle_selection_state;
  volatile u8* battle_selection_lock;

  battle_selection_state = BATTLE_GAME_RAM_BASE;
  battle_selection_lock = BATTLE_LOCK_RAM_BASE;
  if (battle_selection_lock[-0x7a8d] != 0u) {
    return;
  }

  battle_selection_state[0x62e1u] = 1u;
  battle_selection_state[0x62e2u] = 0u;
  battle_selection_state[0x62e3u] = 0u;
  runPanelTasks16To19();
}
