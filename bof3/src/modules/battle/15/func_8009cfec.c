#include "internal.h"

/* possible name: battle_finalize_local_panel_command_band
 * does: deduplicates the staged local panel entry band by owner-kind/panel-id
 * pairs, compacts the live entry array, and writes back the reduced count.
 * @source: 0x8009cfec FUN_8009cfec
 */
void func_8009cfec(void) {
  BattleLocalPanelEntry unique_entries[8];
  u8                    unique_count;
  u8                    source_index;

  unique_count = 0u;
  source_index = 0u;

  if (BOF3_BATTLE_LOCAL_PANEL_ENTRY_COUNT != 0u) {
    do {
      BattleLocalPanelEntry source_entry;
      u8                    duplicate_found;
      u8                    unique_index;

      source_entry = BOF3_BATTLE_LOCAL_PANEL_ENTRY(source_index);
      duplicate_found = 0u;
      unique_index = 0u;

      if (unique_count != 0u) {
        do {
          BattleLocalPanelEntry unique_entry;

          unique_entry = unique_entries[unique_index];
          if ((BOF3_BATTLE_LOCAL_PANEL_OWNER_KIND(source_entry.owner_index) ==
               BOF3_BATTLE_LOCAL_PANEL_OWNER_KIND(unique_entry.owner_index)) &&
              (source_entry.panel_id == unique_entry.panel_id)) {
            duplicate_found = 1u;
            break;
          }

          unique_index += 1u;
        } while (unique_index < unique_count);
      }

      if (duplicate_found == 0u) {
        unique_entries[unique_count] = source_entry;
        unique_count += 1u;
      }

      source_index += 1u;
    } while (source_index < BOF3_BATTLE_LOCAL_PANEL_ENTRY_COUNT);
  }

  BOF3_BATTLE_LOCAL_PANEL_ENTRY_COUNT = unique_count;
  if (unique_count != 0u) {
    source_index = 0u;
    do {
      BOF3_BATTLE_LOCAL_PANEL_ENTRY(source_index) =
          unique_entries[source_index];
      source_index += 1u;
    } while (source_index < BOF3_BATTLE_LOCAL_PANEL_ENTRY_COUNT);
  }
}
