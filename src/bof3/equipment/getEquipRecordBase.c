#include "bof3/core/slus_internal.h"

/* @behavior selects the runtime equipment record base and scales the masked
 * item index by the serialized record stride. Invalid categories fall back to
 * the item table, matching the original dispatch.
 * @source 0x80165D48
 * @see docs/specs/data/equipment.md
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void* getEquipRecordBase(s32 item_type, s32 item_index) {
  u8 category;
  u8 index;

  category = (u8)item_type;
  index = (u8)item_index;
  switch (category) {
    default:
    case 0:
      /* The original dispatch falls through to the item table for all other
       * category values, including the serialized empty sentinel. */
      return (void*)&ITEM_OBJECTS[index];
    case 1:
      return (void*)&WEAPON_OBJECTS[index];
    case 2:
      return (void*)&ARMOR_OBJECTS[index];
    case 3:
      return (void*)&ACCESSORY_OBJECTS[index];
    case 4:
      return (void*)&KEY_ITEM_OBJECTS[index];
  }
}
