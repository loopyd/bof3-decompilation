#include "internal.h"

/* @behavior returns the key-item record for an item-table index.
 * @source 0x800DF5EC
 * @see docs/specs/data/equipment.md
 */
const KeyItemObject* func_800DF5EC(u8 item_type, u8 item_index) {
  (void)item_type;
  return &KEY_ITEM_OBJECTS[item_index & 0xffu];
}
