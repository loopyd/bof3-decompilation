#include "internal.h"

extern GameCallbackSlot* gameCallbackSlotCursor; /* @kind: bss */

/* possible name: game_slot_scheduler_yield
 * @behavior arms the current callback slot with a countdown and forces one
 * scheduler switch through thread id `0xff000000`.
 * @source 0x8014B87C
 */
extern void NO_SIBLING_CALLS func_8014B87C(u16 countdown);

void NO_SIBLING_CALLS func_8014B87C(u16 countdown) {
  GameCallbackSlot* slot;

  slot = gameCallbackSlotCursor;
  slot->countdown = countdown;
  slot->state = GAME_CALLBACK_SLOT_STATE_YIELD;
  ChangeTh(GAME_CALLBACK_FORCE_SWITCH);
}
