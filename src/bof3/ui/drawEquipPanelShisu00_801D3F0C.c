#include "bof3/bof3.h"
#include "data/equipment.h"
#include "gpu/prim.h"

/* Equipment object tables of the shared main executable, whose record strides
 * come from include/data/equipment.h: 0x18, 0x16 and 0x14 bytes.
 * @source 0x801C90DC @kind table */
extern const WeaponObject WEAPON_OBJECTS[];
/* @source 0x801C98A4 @kind table */
extern const ArmorObject ARMOR_OBJECTS[];
/* @source 0x801C9E7C @kind table */
extern const AccessoryObject ACCESSORY_OBJECTS[];

/* Field table of the shared main executable: eight 164-byte records, one per
 * party slot, that the index argument selects and whose equipment bytes it
 * reads. The generated undefined_syms_auto.txt of this target already carries
 * the address, so no local map row is added here.
 * @source 0x80144968 @kind data */
extern u8 D_80144968[];

/* Six-byte per-slot id table of this overlay, in its trailing data blob (bytes
 * 08 0A 09 0B 0C 0C): the id func_801AEA80 arms each of the six equipped-item
 * cells with.
 * @source 0x801D9B5C @kind data */
extern u8 D_801D9B5C[];

/* EQUIP caption of this overlay, eight bytes past that id table in the same
 * blob; handed to func_8014F800 as the address of its first byte.
 * @source 0x801D9B64 @kind data */
extern u8 D_801D9B64[];

/* Panel backing drawer of this overlay, still an `asm` row of this target's
 * Splat layout, so only this call is bound here: it takes the panel origin and
 * the cell counts this site passes as 9 and 10.
 * @source 0x801D7418 */
extern void func_801D7418(s16 arg0, s16 arg1, s32 arg2, s32 arg3);

/* Main-executable panel icon drawer, called here with the icon id 0, the panel
 * cell anchor and the constants 16, 16 and 0x80.
 * @source 0x801644D8 */
extern void func_801644D8(u32 arg0, u32 arg1, u32 arg2, u32 arg3, u32 arg4,
                          u32 arg5);

/* Main-executable text submitter: the pen origin, the alignment, the shade and,
 * as its fifth argument, the glyph-code row it queues.
 * @source 0x8014F800 */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* Main-executable equipment-icon cell helper: one cell at the two
 * zero-extended coordinates with the slot id byte and the caller flag. Both
 * coordinates arrive zero-extended by the andi 0xFFFF of the call site and the
 * id byte is already zero-extended by its lbu load, so those are the call-site
 * widths.
 * @source 0x801AEA80 */
extern void func_801AEA80(u16 arg0, u16 arg1, u8 arg2, s32 arg3);

/* Main-executable equipment-record mask query, called with one of the six
 * item-record pointers this body selected; only the low byte of the answer is
 * used, so the word-wide return is the caller-side narrowing of that byte.
 * @source 0x801AF20C */
extern u32 func_801AF20C(const void* arg0);

/* @source 0x801D3F0C
 * @behavior Equipment panel of the SHISU overlay: it draws the panel backing
 *           through this overlay's drawer func_801D7418 at (x, y) with the size
 *           pair 9/10, selects the 164-byte field record D_80144968[index]
 *           whose item bytes it then reads, draws the 16x16 icon through
 *           func_801644D8 at (x+7, y+5) with the constants 16, 16 and 0x80 and
 *           the EQUIP caption D_801D9B64 through func_8014F800 at (x+0x2E, y+7)
 *           with the mask 0xFF. It next selects the six equipped-item records
 *           of that field record - the weapon record of WEAPON_OBJECTS for
 *           record byte 0x0E, the armor records of ARMOR_OBJECTS for bytes 0x0F
 *           to 0x11 and the accessory records of ACCESSORY_OBJECTS for bytes
 *           0x12 and 0x13 - and for each of the six slots arms the cell at
 *           (x+5, y+13*slot+0x1C) through func_801AEA80 with the per-slot id
 *           byte of D_801D9B5C and 0, then submits that record again through
 *           func_8014F800 at (x+0x11, y+13*slot+0x1A) with the low byte of the
 *           mask func_801AF20C reports for it. Nothing is returned and the
 *           0x50-byte frame keeps $s0 to $s5 and $ra across the calls. No jal
 *           or table word of this payload names the span, so its three
 *           arguments have the roles their uses give them: a caller outside the
 *           payload invokes it. One payload cmp proved these 0x24C bytes equal
 *           the drawEquipPanelBate00_801D2F64 span at 0x801D2F64 apart from
 *           three words - the jal at +0x40 to this overlay's own panel drawer
 *           func_801D7418 where the BATE copy calls func_801D6470, and the two
 *           literal low halves at +0x6C (caption D_801D9B64) and +0x1D0 (id
 *           table D_801D9B5C) - and the same three words are the only
 *           difference from the emi/etc/shop/00 span at 0x801D8A0C, so this is
 *           one body with this overlay's bindings.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The body follows the BATE copy of this panel: the slot row offset stays a
 * u16 local that is not narrowed until its uses, so the armed cell takes
 * (u16)(row + 2) and the submitted row the (s16)row truncation.
 */
void drawEquipPanelShisu00_801D3F0C(s16 x, s16 y, u8 index) {
  u8* record;
  const void* cells[6];
  u8 slot;
  u32 mask;
  u16 row;

  func_801D7418(x, y, 9, 10);
  record = &D_80144968[index * 164];
  func_8014F800((s16)(x + 0x2E), (s16)(y + 7), 0, 0xFF, (u32)D_801D9B64);
  func_801644D8(0, (u16)(x + 7), (u16)(y + 5), 16, 16, 128);
  cells[0] = &WEAPON_OBJECTS[record[0x0E]];
  cells[1] = &ARMOR_OBJECTS[record[0x0F]];
  cells[2] = &ARMOR_OBJECTS[record[0x10]];
  cells[3] = &ARMOR_OBJECTS[record[0x11]];
  cells[4] = &ACCESSORY_OBJECTS[record[0x12]];
  cells[5] = &ACCESSORY_OBJECTS[record[0x13]];
  for (slot = 0; slot < 6; slot++) {
    row = slot * 13 + 0x1A + y;
    func_801AEA80((u16)(x + 5), (u16)(row + 2), D_801D9B5C[slot], 0);
    mask = func_801AF20C(cells[slot]);
    func_8014F800((s16)(x + 0x11), (s16)row, 0, mask & 0xFF, (u32)cells[slot]);
  }
}
