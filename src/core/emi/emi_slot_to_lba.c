#include "internal.h"

/* possible name: emi_slot_to_lba
 * does: resolves a top-level BOF3 slot id into the matching disc LBA.
 */
u32 emi_slot_to_lba(const u32* slot_lba_table, size_t slot_count, u32 slot_id) {
  if (slot_lba_table == NULL || slot_id >= slot_count) {
    return 0;
  }

  return slot_lba_table[slot_id];
}
