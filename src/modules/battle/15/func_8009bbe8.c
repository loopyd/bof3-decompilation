#include "internal.h"

/* possible name: battle_refresh_local_panel_command_band
 * @behavior walks the four current battler panel-rule rows, validates each rule
 * against the live battler state, stages accepted entries into the local panel
 * slots, and finalizes the panel band refresh.
 * @source 0x8009bbe8 FUN_8009bbe8
 */
void func_8009bbe8(void) {
  volatile u8* current_battler;
  volatile u8* panel_rule;
  u8           slot_index;
  u8           condition_ready;

  slot_index = 0u;
  do {
    current_battler = BATTLE_CURRENT_BATTLER_PTR;
    panel_rule = BATTLE_LOCAL_PANEL_RULE(current_battler[0xe0], slot_index);
    condition_ready = 0u;

    switch (panel_rule[0]) {
      case 0:
        condition_ready = func_8009c8ac(1u);
        break;

      case 1:
        condition_ready = func_8009c8ac(2u);
        break;

      case 2:
        condition_ready = func_8009c8ac(4u);
        break;

      case 3:
        condition_ready = func_8009c8ac(8u);
        break;

      case 4:
        condition_ready = func_8009c8ac(0x10u);
        break;

      case 5:
        condition_ready = func_8009c8ac(0x20u);
        break;

      case 6:
        condition_ready = func_8009c8ac(0x40u);
        break;

      case 7:
        condition_ready = func_8009c8ac(0x100u);
        break;

      case 8:
        condition_ready = func_8009c8ac(0x80u);
        break;

      case 9:
        if (BATTLE_PANEL_RULE_PASS_KIND == 4u) {
          current_battler = BATTLE_CURRENT_BATTLER_PTR;
          if (*(volatile s16*)(current_battler + 0xf8) != 0) {
            condition_ready = 1u;
          }
        }
        break;

      case 10:
        if (BATTLE_PANEL_RULE_PASS_KIND == 1u) {
          current_battler = BATTLE_CURRENT_BATTLER_PTR;
          if (*(volatile s16*)(current_battler + 0xf8) != 0) {
            condition_ready = 1u;
          }
        }
        break;

      case 0x16:
        current_battler = BATTLE_CURRENT_BATTLER_PTR;
        if ((*(volatile u16*)(current_battler + 0x82) & 8u) != 0u) {
          condition_ready = 1u;
        }
        break;

      case 0x17:
        current_battler = BATTLE_CURRENT_BATTLER_PTR;
        if ((*(volatile u16*)(current_battler + 0x82) & 0x80u) != 0u) {
          condition_ready = 1u;
        }
        break;

      case 0x18:
        current_battler = BATTLE_CURRENT_BATTLER_PTR;
        if (current_battler[0x9a] == 0u) {
          condition_ready = 1u;
        }
        break;

      case 0x21:
        if (func_8009c8ac(1u) != 0u) {
          current_battler = BATTLE_CURRENT_BATTLER_PTR;
          battle_copy_local_panel_rule_entry(current_battler, panel_rule);
        }
        slot_index += 1u;
        continue;

      case 0x22:
        if (func_8009c8ac(2u) != 0u) {
          current_battler = BATTLE_CURRENT_BATTLER_PTR;
          battle_copy_local_panel_rule_entry(current_battler, panel_rule);
        }
        slot_index += 1u;
        continue;

      case 0x23:
        if (func_8009c8ac(4u) != 0u) {
          current_battler = BATTLE_CURRENT_BATTLER_PTR;
          battle_copy_local_panel_rule_entry(current_battler, panel_rule);
        }
        slot_index += 1u;
        continue;

      case 0x24:
        if (BATTLE_PANEL_RULE_PASS_KIND == 1u) {
          current_battler = BATTLE_CURRENT_BATTLER_PTR;
          if (*(volatile s16*)(current_battler + 0xf8) != 0) {
            battle_copy_local_panel_rule_entry(current_battler, panel_rule);
          }
        }
        slot_index += 1u;
        continue;

      case 0x25:
        current_battler = BATTLE_CURRENT_BATTLER_PTR;
        if (*(volatile u16*)(current_battler + 0x94) <=
            *(volatile s16*)(current_battler + 0xf8)) {
          condition_ready = 1u;
        }
        break;

      default:
        break;
    }

    if (condition_ready != 0u) {
      current_battler = BATTLE_CURRENT_BATTLER_PTR;
      if (battle_local_panel_slot_has_entry(current_battler, slot_index) ==
          0u) {
        battle_copy_local_panel_rule_entry(current_battler, panel_rule);
        battle_set_local_panel_slot_active(current_battler, slot_index, 1u);
      }
    }

    slot_index += 1u;
  } while (slot_index < 4u);

  func_8009cfec();
}
