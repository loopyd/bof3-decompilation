#include "internal.h"

/* @behavior resolves one selected native loader slot to authored shipped-file
 * metadata.
 * @source D_80182444 is the native u32 LBA table; this helper is authored.
 */
const SlotTableEntry* slot_table_find(EmiLoaderSlotId slot_id) {
  size_t index;

  for (index = 0; index < g_slot_table_count; ++index) {
    if (g_slot_table[index].slot_id == slot_id) {
      return &g_slot_table[index];
    }
  }

  return NULL;
}
