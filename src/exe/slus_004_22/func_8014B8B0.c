#include "internal.h"

extern GameCallbackSlot* gameCallbackSlotCursor; /* @kind: bss */

/* possible name: game_exit_current_callback_thread
 * @behavior clears the current slot state, closes its thread inside the scheduler
 * critical section, then forces a scheduler switch away from the current
 * callback.
 * @source 0x8014B8B0
 */
void NO_SIBLING_CALLS func_8014B8B0(void) {
  gameCallbackSlotCursor->state = GAME_CALLBACK_SLOT_STATE_EMPTY;
  EnterCriticalSection();
  CloseTh(gameCallbackSlotCursor->thread_id);
  ExitCriticalSection();
  ChangeTh(GAME_CALLBACK_FORCE_SWITCH);
}
