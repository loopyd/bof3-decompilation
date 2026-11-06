#include "bof3/core/slus_internal.h"
#include "bof3/ui/shop00_internal.h"

/* @source 0x801D8A0C
 * @behavior shop equipment panel reached from the panel state dispatcher
 *           func_801E2DB4 with the panel task root x and field-6 halfwords and
 *           the field-record index D_80181B10[D_80144F5A[task byte 0xA]]: it
 *           draws the panel backing through func_801DBF18 with the size pair
 *           9/10, the 16x16 icon through func_801644D8 at (x+7, y+5) with the
 *           constants 16, 16 and 0x80, and the EQUIP caption through
 *           func_8014F800 at (x+0x2E, y+7) with the mask 0xFF and the caption
 *           string D_801E583C. It then selects the six equipped-item records of
 *           the 164-byte field record the index byte chooses - the weapon record
 *           of WEAPON_OBJECTS for record byte 0x0E, the armor records of
 *           ARMOR_OBJECTS for bytes 0x0F..0x11 and the accessory records of
 *           ACCESSORY_OBJECTS for bytes 0x12..0x13 - and for each of the six
 *           slots arms the cell at (x+5, y+13*slot+0x1C) through func_801AEA80
 *           with the per-slot id byte of D_801E5834 and 0, then submits the
 *           record again through func_8014F800 at (x+0x11, y+13*slot+0x1A) with
 *           the low byte of the mask func_801AF20C reports for it.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The slot row offset is a named u16 local. `row = slot * 13 + 0x1A + y`
 * rebuilds the original's chain in that order - the product plus 0x1A, then
 * plus y, then plus 2 for the cell anchor - and keeps the value un-narrowed
 * until each use, so the armed cell takes `(u16)(row + 2)` and the submitted
 * row takes the `sll`/`sra` narrowing of `(s16)row`. Two measured alternatives
 * lose the shape: the inline `y + slot * 13 + 0x1A + 2` spelling folds the two
 * constants into one 0x1C add and materializes the 0x1A after the record query,
 * two instructions short, and an s32 row local loses the whole chain.
 */
void drawShopEquipPanel(s16 x, s16 y, u8 index) {
  u8* record;
  const void* cells[6];
  u8 slot;
  u32 mask;
  u16 row;

  func_801DBF18(x, y, 9, 10);
  record = &D_80144968[index * 164];
  func_8014F800((s16)(x + 0x2E), (s16)(y + 7), 0, 0xFF, (u32)D_801E583C);
  func_801644D8(0, (u16)(x + 7), (u16)(y + 5), 16, 16, 128);
  cells[0] = &WEAPON_OBJECTS[record[0x0E]];
  cells[1] = &ARMOR_OBJECTS[record[0x0F]];
  cells[2] = &ARMOR_OBJECTS[record[0x10]];
  cells[3] = &ARMOR_OBJECTS[record[0x11]];
  cells[4] = &ACCESSORY_OBJECTS[record[0x12]];
  cells[5] = &ACCESSORY_OBJECTS[record[0x13]];
  for (slot = 0; slot < 6; slot++) {
    row = slot * 13 + 0x1A + y;
    func_801AEA80((u16)(x + 5), (u16)(row + 2), D_801E5834[slot], 0);
    mask = func_801AF20C(cells[slot]);
    func_8014F800((s16)(x + 0x11), (s16)row, 0, mask & 0xFF, (u32)cells[slot]);
  }
}
