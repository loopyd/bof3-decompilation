#include "bof3/context.h"
#include "data/equipment.h"

/* The four shared equipment object tables owned by the main executable. The
 * strides the original scales its index by come from the record types in
 * include/data/equipment.h: 0x12, 0x18, 0x16 and 0x14 bytes. */
/* @source 0x801C8964 @kind table */
extern const ItemObject ITEM_OBJECTS[];
/* @source 0x801C90DC @kind table */
extern const WeaponObject WEAPON_OBJECTS[];
/* @source 0x801C98A4 @kind table */
extern const ArmorObject ARMOR_OBJECTS[];
/* @source 0x801C9E7C @kind table */
extern const AccessoryObject ACCESSORY_OBJECTS[];

/* The shared twelve-byte name slot the overlays copy equipment object names
 * into; the bounded string copy below writes at most 0xC bytes plus a
 * terminator, which is the length of the name field of every object type. */
/* @source 0x801490D8 @kind data */
extern u8 D_801490D8[];

/* Bounded string copy at 0x801501E4: copies up to the byte count from the
 * source, stopping at a NUL, and terminates the destination. */
void func_801501E4(void* destination, u32 source, u32 count);

/* @source 0x801EEF98
 * @behavior copies the name of the equipment object selected by the masked
 *           category and index pair into the shared name slot at 0x801490D8
 *           with the bounded count 0xC. Categories 1, 2 and 3 select the
 *           weapon, armor and accessory tables with their own record strides
 *           0x18, 0x16 and 0x14; the default arm, which also takes the
 *           serialized empty category 0, selects the item table with stride
 *           0x12. The original dispatch is a compare chain over the category
 *           byte and duplicates the copy call in every arm.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyEquipmentObjectNameBmagicMagic21600_801EEF98(s32 item_index, s32 item_type) {
  u8 index;
  u8 category;

  index = (u8)item_index;
  category = (u8)item_type;
  switch (category) {
    default:
    case 0:
      /* The original dispatch falls through to the item table for every other
       * category value, including the serialized empty sentinel. */
      func_801501E4(D_801490D8, (u32)&ITEM_OBJECTS[index], 0xCu);
      break;
    case 1:
      func_801501E4(D_801490D8, (u32)&WEAPON_OBJECTS[index], 0xCu);
      break;
    case 2:
      func_801501E4(D_801490D8, (u32)&ARMOR_OBJECTS[index], 0xCu);
      break;
    case 3:
      func_801501E4(D_801490D8, (u32)&ACCESSORY_OBJECTS[index], 0xCu);
      break;
  }
}
