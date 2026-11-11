#include "bof3/bof3.h"
#include "data/equipment.h"

/* Field table of the main executable, outside this EMI payload: eight 164-byte
 * records, one per party slot, whose bytes 0x0E through 0x13 hold the six
 * equipped ids this body draws.
 * @source 0x80144968 @kind table */
extern u8 D_80144968[];

/* Equipment object tables of the main executable, whose record strides come
 * from include/data/equipment.h: 0x18, 0x16 and 0x14 bytes.
 * @source 0x801C90DC @kind table */
extern const WeaponObject WEAPON_OBJECTS[];
/* @source 0x801C98A4 @kind table */
extern const ArmorObject ARMOR_OBJECTS[];
/* @source 0x801C9E7C @kind table */
extern const AccessoryObject ACCESSORY_OBJECTS[];

/* Inset rectangle painter of the main executable: it fills the width by height
 * area at the two coordinates with the shade its fifth argument carries. This
 * payload only calls it.
 * @source 0x801AED54 */
extern void func_801AED54(s32 x, s32 y, s32 width, s32 height, s32 shade);

/* Shared text submitter of the main executable: the pen origin, the alignment,
 * the cell count and, as the fifth argument, the record address it queues.
 * @source 0x8014F800 */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* Palette cell emitter of the main executable: one cell at the two
 * zero-extended coordinates with the palette byte and the caller flag.
 * @source 0x801AEA80 */
extern void func_801AEA80(u16 arg0, u16 arg1, u8 arg2, s32 arg3);

/* @source 0x801D3B4C
 * @behavior Equipment-slot block of the BATE overlay: it paints the inset
 *   backing rectangle of the block through func_801AED54 at (x+2, y+0x44) with
 *   the size 0x87 by 0x58 and the shade 0xF1, then draws the six equipped slots
 *   of the 164-byte field record the index byte selects out of D_80144968 - the
 *   weapon record of WEAPON_OBJECTS for record byte 0x0E, the armor records of
 *   ARMOR_OBJECTS for bytes 0x0F through 0x11 and the accessory records of
 *   ACCESSORY_OBJECTS for bytes 0x12 and 0x13. Each slot is submitted through
 *   func_8014F800 at (x+0x0F, y+0x46/0x53/0x60/0x6D/0x81/0x8E) as an
 *   eight-cell run, aligned to 7 when the flag byte is nonzero and to 0 when it
 *   is zero; the six palette cells 8, 0xA, 9, 0xB, 0xC and 0xC follow at (x+5,
 *   y+0x49/0x56/0x63/0x70/0x84/0x91) through func_801AEA80 with that same flag
 *   byte as the fourth argument. The payload names this span only from the jal
 *   at 0x801D5EFC inside func_801D5E90, which passes the task origin halfwords,
 *   the leading byte of the 0x140-byte record D_80145FCC[task byte 0xC] and the
 *   low byte of its own second argument, right after painting the panel backing
 *   through func_801AE3F0 and right before the jal it makes to func_801D37AC
 *   for the same arguments. Its 0x38-byte frame keeps $s0 through $s6 and $ra.
 *   One payload cmp proved these 0x2CC bytes equal
 *   drawEquipSlotRowsShisu00_801D4AF4 at 0x801D4AF4 in emi/etc/shisu/00, lifted
 *   exact there, so this body carries no target-specific literal.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawEquipSlotRowsBate00_801D3B4C(s16 x, s16 y, u8 index, u8 flag) {
  u8* record;
  u8 align;

  align = flag ? 7 : 0;
  func_801AED54((s16)(x + 2), (s16)(y + 0x44), 0x87, 0x58, 0xF1);
  record = &D_80144968[index * 164];
  func_8014F800((s16)(x + 0xF), (s16)(y + 0x46), align, 8,
                (u32)&WEAPON_OBJECTS[record[0x0E]]);
  func_8014F800((s16)(x + 0xF), (s16)(y + 0x53), align, 8,
                (u32)&ARMOR_OBJECTS[record[0x0F]]);
  func_8014F800((s16)(x + 0xF), (s16)(y + 0x60), align, 8,
                (u32)&ARMOR_OBJECTS[record[0x10]]);
  func_8014F800((s16)(x + 0xF), (s16)(y + 0x6D), align, 8,
                (u32)&ARMOR_OBJECTS[record[0x11]]);
  func_8014F800((s16)(x + 0xF), (s16)(y + 0x81), align, 8,
                (u32)&ACCESSORY_OBJECTS[record[0x12]]);
  func_8014F800((s16)(x + 0xF), (s16)(y + 0x8E), align, 8,
                (u32)&ACCESSORY_OBJECTS[record[0x13]]);
  func_801AEA80((u16)(x + 5), (u16)(y + 0x49), 8, flag);
  func_801AEA80((u16)(x + 5), (u16)(y + 0x56), 0xA, flag);
  func_801AEA80((u16)(x + 5), (u16)(y + 0x63), 9, flag);
  func_801AEA80((u16)(x + 5), (u16)(y + 0x70), 0xB, flag);
  func_801AEA80((u16)(x + 5), (u16)(y + 0x84), 0xC, flag);
  func_801AEA80((u16)(x + 5), (u16)(y + 0x91), 0xC, flag);
}
