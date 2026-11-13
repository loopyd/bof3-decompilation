#include "bof3/core/slus_internal.h"

/* This authored helper finds shipped-file metadata for a selected loader slot.
 * emiSlotLbaTable is the native u32 LBA table. */
const SlotTableEntry* slot_table_find(EmiLoaderSlotId slot_id) {
  size_t index;

  for (index = 0; index < g_slot_table_count; ++index) {
    if (g_slot_table[index].slot_id == slot_id) {
      return &g_slot_table[index];
    }
  }

  return NULL;
}
