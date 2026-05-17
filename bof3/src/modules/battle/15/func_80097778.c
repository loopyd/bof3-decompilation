#include "internal.h"

/* does: chooses the first result-side substate from the committed selection
 * kind flags, refreshes the active slot family byte, clears the local
 * ring-reset halfword, and sets the shared pending-kind byte.
 * @source: 0x80097778 FUN_80097778
 */
void func_80097778(void) {
  volatile u8* active_selection_slot;
  u8           selected_kind_flags;

  active_selection_slot = BATTLE_ACTIVE_SELECTION_SLOT_PTR;
  selected_kind_flags = BATTLE_SELECTION_KIND_FLAGS(
      *(volatile u16*)(active_selection_slot + 2));

  if ((selected_kind_flags & 0x20u) != 0u) {
    active_selection_slot[0] = battle_resolve_selection_slot(3u);
    BATTLE_SELECTION_SUBSTATE = 1u;
  } else {
    if (battle_result_uses_empty_slot() != 0u) {
      active_selection_slot[0] = 0u;
    } else {
      active_selection_slot[0] = battle_resolve_selection_slot(0u);
    }
    BATTLE_SELECTION_SUBSTATE = 2u;
  }

  BATTLE_SELECTION_RING_RESET = 0u;
  BATTLE_SELECTION_PENDING_KIND = 1u;
}
