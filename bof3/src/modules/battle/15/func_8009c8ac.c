#include "internal.h"

/* possible name: battle_panel_condition_mask_is_ready
 * does: validates one local panel rule mask against the current battler state
 * and the active panel-rule pass mode bytes.
 * @source: 0x8009c8ac FUN_8009c8ac
 */
u8 func_8009c8ac(u16 required_mask) {
  volatile u8* current_battler;
  u8           panel_slot_kind;

  current_battler = BATTLE_CURRENT_BATTLER_PTR;
  if (*(volatile s16*)(current_battler + 0xf8) == 0) {
    return 0u;
  }

  if (BATTLE_PANEL_RULE_PASS_KIND == 4u) {
    if ((BATTLE_SELECTION_KIND_MASK(
             BATTLE_PANEL_RULE_PASS_SELECTION) &
         required_mask) != 0u) {
      return 1u;
    }
  }

  if (BATTLE_PANEL_RULE_PASS_KIND != 1u) {
    return 0u;
  }

  if (BATTLE_PANEL_RULE_PASS_SLOT >= 3u) {
    return 0u;
  }

  if ((required_mask & 0x100u) != 0u) {
    return 0u;
  }

  panel_slot_kind =
      BATTLE_PANEL_SLOT_KIND(BATTLE_PANEL_RULE_PASS_SLOT);
  if ((BATTLE_PANEL_SLOT_MASK(panel_slot_kind) & (u8)required_mask) !=
      0u) {
    return 1u;
  }

  return 0u;
}
