#include "internal.h"

extern GameCallbackSlot* D_80143D40;

/* possible name: game_exit_current_callback_thread
 * @behavior clears the current slot state, closes its thread inside the scheduler
 * critical section, then forces a scheduler switch away from the current
 * callback.
 * @source 0x8014B8B0
 */
void NO_SIBLING_CALLS func_8014B8B0(void) {
  D_80143D40->state = GAME_CALLBACK_SLOT_STATE_EMPTY;
  func_8017EE0C();
  CloseTh(D_80143D40->thread_id);
  func_8017EE1C();
  ChangeTh(GAME_CALLBACK_FORCE_SWITCH);
}
