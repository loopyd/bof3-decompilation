#include "bof3/core/slus_internal.h"

/* This authored helper resolves a top-level slot ID to its disc LBA. */
u32 emiSlotToLba(const u32* slot_lba_table, size_t slot_count, u32 slot_id) {
  if (slot_lba_table == NULL || slot_id >= slot_count) {
    return 0;
  }

  return slot_lba_table[slot_id];
}
