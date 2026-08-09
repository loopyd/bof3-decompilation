#include "bof3/core/slus_internal.h"

/* @behavior arms the current callback slot with a countdown and forces one
 * scheduler switch through thread id `0xff000000`.
 * @source 0x8014B87C
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */

/* @source 0x80143D40 @kind bss */
extern GameCallbackSlot* gameCallbackSlotCursor;

extern void NO_SIBLING_CALLS yieldCallbackSlotScheduler(u16 countdown);

void NO_SIBLING_CALLS yieldCallbackSlotScheduler(u16 countdown) {
  GameCallbackSlot* slot;

  slot = gameCallbackSlotCursor;
  slot->countdown = countdown;
  slot->state = GAME_CALLBACK_SLOT_STATE_YIELD;
  ChangeTh(GAME_CALLBACK_FORCE_SWITCH);
}
