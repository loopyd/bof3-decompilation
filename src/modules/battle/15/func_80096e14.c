#include "internal.h"

/* does: begins the slot-selection input branch, arms the active slot/message
 * side bytes, clears the local selection tuple, and advances the shared battle
 * progression byte.
 * @source: 0x80096e14 FUN_80096e14
 */
void func_80096e14(void) {
  volatile u8* battle_selection_state;
  u8           advance_counter;

  battle_selection_state = (volatile u8*)0x80140000u;
  battle_queue_frontend_cue(0x104u);
  battle_selection_state[0x62efu] = 0u;
  BATTLE_ACTIVE_SELECTION_SLOT_PTR[1] = 1u;
  ((volatile u8*)BATTLE_ACTIVE_MESSAGE_SLOT_PTR)[1] = 2u;
  advance_counter = battle_selection_state[0x6303u];
  battle_selection_state[0x62e1u] = 1u;
  battle_selection_state[0x62e2u] = 0u;
  battle_selection_state[0x62e3u] = 0u;
  battle_selection_state[0x62e4u] = 0u;
  battle_selection_state[0x6303u] = advance_counter + 1u;
}
