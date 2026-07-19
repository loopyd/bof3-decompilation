#include "internal.h"

/* @behavior arms the active slot for the secondary grid, initializes the
 * secondary-selection grid scratch band, then advances back into the shared
 * root update.
 * @source 0x80098408
 */
void func_80098408(void) {
  u8* active_selection_slot;
  u8* battle_selection_state;
  u8  selection_root_state;

  active_selection_slot = (u8*)BATTLE_ACTIVE_SELECTION_SLOT_PTR;
  battle_selection_state = (u8*)BATTLE_GAME_RAM_BASE;
  active_selection_slot[1] = 5u;
  func_8009AF84();
  selection_root_state = battle_selection_state[0x62e3u];
  battle_selection_state[0x62e4u] = 0u;
  battle_selection_state[0x62e3u] = selection_root_state + 1u;
}
