#include "bof3/context.h"
#include "data/equipment.h"

/* The 164-byte field-record table of the shared main executable. The index byte
 * of this block selects one party-member record, and the six equipment ids sit
 * at its bytes 0x0E through 0x13. The generated undefined_syms_auto.txt of this
 * target already carries this address, so no local map row is added here.
 * @source 0x80144968 @kind table */
extern u8 D_80144968[];

/* The equipment object tables of the shared main executable, whose record
 * strides come from include/data/equipment.h: 0x18, 0x16 and 0x14 bytes.
 * @source 0x801C90DC @kind table */
extern const WeaponObject WEAPON_OBJECTS[];
/* @source 0x801C98A4 @kind table */
extern const ArmorObject ARMOR_OBJECTS[];
/* @source 0x801C9E7C @kind table */
extern const AccessoryObject ACCESSORY_OBJECTS[];

/* Insets rectangle painter of the main executable: it fills the width by height
 * area at the two coordinates with the shade its fifth argument carries. This
 * payload only calls it.
 * @source 0x801AED54 */
extern void func_801AED54(s32 x, s32 y, s32 width, s32 height, s32 shade);

/* Shared text submitter of the main executable: it takes the pen origin, the
 * alignment, the cell count and, as its fifth argument, the glyph-code row it
 * queues. This payload only calls it.
 * @source 0x8014F800 */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* Palette cell emitter of the main executable: one cell at the two
 * zero-extended coordinates with the palette byte and the caller flag. This
 * payload only calls it.
 * @source 0x801AEA80 */
extern void func_801AEA80(u16 arg0, u16 arg1, u8 arg2, s32 arg3);

/* @source 0x801D95F4
 * @behavior Equipment-slot block of the SHOP overlay: it paints the inset
 *   backing rectangle of the block through func_801AED54 at (x+2, y+0x44) with
 *   the size 0x87 by 0x58 and the shade 0xF1, then draws the six equipped slots
 *   of the 164-byte field record the index byte selects out of D_80144968 - the
 *   weapon record of WEAPON_OBJECTS for record byte 0x0E, the armor records of
 *   ARMOR_OBJECTS for bytes 0x0F through 0x11 and the accessory records of
 *   ACCESSORY_OBJECTS for bytes 0x12 and 0x13. Each slot name is submitted
 *   through func_8014F800 at (x+0x0F, y+0x46/0x53/0x60/0x6D/0x81/0x8E) as an
 *   eight-cell run, aligned to 7 when the flag byte is nonzero and to 0 when it
 *   is zero; the six palette cells 8, 0xA, 9, 0xB, 0xC and 0xC follow at
 *   (x+5, y+0x49/0x56/0x63/0x70/0x84/0x91) through func_801AEA80 with that same
 *   flag byte as the fourth argument. This payload names the span only from the
 *   jal at 0x801DB9A4 inside func_801DB938, which passes the task x and field-6
 *   halfwords, the leading byte of the 0x140-byte record D_80145FCC[task byte
 *   0xC] and the low byte of its own second argument, right after painting the
 *   panel backing through func_801AE3F0. Its 0x38-byte frame keeps $s0 through
 *   $s6 and $ra. One payload cmp proved these 0x2CC bytes equal
 *   drawEquipSlotRowsShisu00_801D4AF4 at 0x801D4AF4 in emi/etc/shisu/00 and
 *   drawEquipSlotRowsBate00_801D3B4C at 0x801D3B4C in emi/etc/bate/00, so the
 *   statement order, operand forms and declaration style below are those of
 *   those exact siblings, and every literal and symbol binding below is read
 *   from this span itself.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawEquipSlotRowsShop00_801D95F4(u16 x, u16 y, u8 index, u8 flag) {
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
