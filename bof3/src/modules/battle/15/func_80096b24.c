#include "internal.h"

/* does: seeds the active selection slot, arms the pending-kind byte, clears the
 * local ring-reset halfword, and advances the slot-selection substate byte.
 * @source: 0x80096b24 FUN_80096b24
 */
void func_80096b24(void) {
  *BOF3_BATTLE_ACTIVE_SELECTION_SLOT_PTR = battle_resolve_selection_slot(3u);
  BOF3_BATTLE_SELECTION_PENDING_KIND = 1u;
  BOF3_BATTLE_SELECTION_RING_RESET = 0u;
  BOF3_BATTLE_SELECTION_SUBSTATE += 1u;
}
