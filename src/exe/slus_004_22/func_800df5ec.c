#include "internal.h"

/* @behavior returns the key-item record for an item-table index.
 * @source 0x800df5ec
 * @see docs/specs/data/equipment.md
 */
const KeyItemObject* func_800df5ec(u8 item_type, u8 item_index) {
  (void)item_type;
  return &KEY_ITEM_OBJECTS[item_index & 0xffu];
}
