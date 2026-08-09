#include "bof3/core/slus_internal.h"

/* @behavior clears the current slot state, closes its thread inside the scheduler
 * critical section, then forces a scheduler switch away from the current
 * callback.
 * @source 0x8014B8B0
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */

/* @source 0x80143D40 @kind bss */
extern GameCallbackSlot* gameCallbackSlotCursor;

void NO_SIBLING_CALLS exitCurrentCallbackThread(void) {
  gameCallbackSlotCursor->state = GAME_CALLBACK_SLOT_STATE_EMPTY;
  EnterCriticalSection();
  CloseTh(gameCallbackSlotCursor->thread_id);
  ExitCriticalSection();
  ChangeTh(GAME_CALLBACK_FORCE_SWITCH);
}
