#include "internal.h"

/* @behavior selects the runtime equipment record base and scales the masked
 * item index by the serialized record stride. Invalid categories fall back to
 * the item table, matching the original dispatch.
 * @source 0x800DF548
 * @see docs/specs/data/equipment.md
 */
void* func_800DF548(s32 item_type, s32 item_index) {
  u8 category;
  u8 index;

  category = (u8)item_type;
  index = (u8)item_index;
  switch (category) {
    case 0:
      return (void*)&ITEM_OBJECTS[index];
    case 1:
      return (void*)&WEAPON_OBJECTS[index];
    case 2:
      return (void*)&ARMOR_OBJECTS[index];
    case 3:
      return (void*)&ACCESSORY_OBJECTS[index];
    case 4:
      return (void*)&KEY_ITEM_OBJECTS[index];
    default:
      /* The original dispatch falls through to the item table for all other
       * category values, including the serialized empty sentinel. */
      return (void*)&ITEM_OBJECTS[index];
  }
}
