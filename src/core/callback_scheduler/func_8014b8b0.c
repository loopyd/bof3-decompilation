#include "internal.h"

extern GameCallbackSlot* DAT_80143d40;

/* possible name: game_exit_current_callback_thread
 * does: clears the current slot state, closes its thread inside the scheduler
 * critical section, then forces a scheduler switch away from the current
 * callback.
 * @source: 0x8014b8b0 FUN_8014b8b0
 * @source: docs/specs/runtime/game-overlay.md
 * @source: processed/inventory/inventory.sqlite (function metadata and refs)
 */
void NO_SIBLING_CALLS func_8014b8b0(void) {
  DAT_80143d40->state = GAME_CALLBACK_SLOT_STATE_EMPTY;
  func_8017ee0c();
  func_8017edac(DAT_80143d40->thread_id);
  func_8017ee1c();
  ChangeTh(GAME_CALLBACK_FORCE_SWITCH);
}
