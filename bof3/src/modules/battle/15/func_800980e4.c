#include "internal.h"

/* does: reseeds the active selection slot, stages attack-name slot 2, clears
 * the local ring-reset halfword, and advances the top-level selection state.
 * @source: 0x800980e4 FUN_800980e4
 */
void func_800980e4(void) {
  *BATTLE_ACTIVE_SELECTION_SLOT_PTR = battle_resolve_selection_slot(3u);
  battle_stage_attack_name_message(2, 0);
  BATTLE_SELECTION_PENDING_KIND = 1u;
  BATTLE_SELECTION_RING_RESET = 0u;
  BATTLE_SELECTION_ROOT_STATE += 1u;
}
