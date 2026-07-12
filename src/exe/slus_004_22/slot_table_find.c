#include "internal.h"

/* @behavior resolves one proven BOF3 runtime slot to the original shipped file.
 * @source 0x80182444 DAT_80182444
 */
const SlotTableEntry* slot_table_find(u32 slot_id) {
  size_t index;

  for (index = 0; index < g_slot_table_count; ++index) {
    if (g_slot_table[index].slot_id == slot_id) {
      return &g_slot_table[index];
    }
  }

  return NULL;
}
