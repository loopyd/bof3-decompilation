#include "internal.h"

/* clang-format off */
#include <libapi.h>
/* clang-format on */

extern GameCallbackSlot* DAT_80143d40;

/* possible name: game_slot_scheduler_yield
 * does: arms the current callback slot with a countdown and forces one
 * scheduler switch through thread id `0xff000000`.
 * @source: 0x8014b87c FUN_8014b87c
 * @source: docs/specs/runtime/game-overlay.md
 * @source: processed/inventory/inventory.sqlite (function metadata and refs)
 */
void BOF3_NO_SIBLING_CALLS func_8014b87c(u16 countdown);

void BOF3_NO_SIBLING_CALLS func_8014b87c(u16 countdown) {
  GameCallbackSlot* slot;

  slot = DAT_80143d40;
  slot->countdown = countdown;
  slot->state = BOF3_GAME_CALLBACK_SLOT_STATE_YIELD;
  ChangeTh(BOF3_GAME_CALLBACK_FORCE_SWITCH);
}
