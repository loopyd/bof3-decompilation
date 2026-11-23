#include "bof3/bof3.h"
#include "data/equipment.h"
#include "gpu/prim.h"

/* Shared CLUT-bank work block of the main executable, viewed at the byte every
 * SHISU panel passes as the trailing argument of its inset painter: offset 0 of
 * this block holds that CLUT-bank byte, and the 164-byte field records begin at
 * block offset 0x16. The original derives the record base from the same
 * materialised symbol address with one addiu of 0x16, so both sites name this
 * one block. The generated undefined_syms_auto.txt of this target already
 * carries these addresses, so no local map row is added here.
 * @source 0x80144952 @kind table */
extern u8 D_80144952[];

/* The equipment object tables of the shared main executable, whose record
 * strides come from include/data/equipment.h: 0x18, 0x16 and 0x14 bytes.
 * @source 0x801C90DC @kind table */
extern const WeaponObject WEAPON_OBJECTS[];
/* @source 0x801C98A4 @kind table */
extern const ArmorObject ARMOR_OBJECTS[];
/* @source 0x801C9E7C @kind table */
extern const AccessoryObject ACCESSORY_OBJECTS[];

/* Shared sprintf work buffer of the main executable, outside this EMI payload:
 * every formatted value row is written here and then handed to the line-text
 * drawer as the buffer address.
 * @source 0x80145AD4 @kind data */
extern u8 D_80145AD4[];

/* Three-cell format of this overlay's leading data blob at payload byte 4: the
 * "%3d" string every formatted value row of this body uses.
 * @source 0x801D0C04 @kind data */
extern const char D_801D0C04[];

/* Glyph-code run of this overlay's trailing data blob at payload byte 0x9118:
 * four records of six bytes each, one per row of the caption grid this body
 * submits through func_8014F800. Only this body reads the run, so it keeps the
 * generated name of its address.
 * @source 0x801D9D18 @kind data */
extern u8 D_801D9D18[];

/* Per-slot palette byte of the six equipment rows, indexed by the slot number.
 * Only this body reads the run.
 * @source 0x801D9D30 @kind table */
extern u8 D_801D9D30[];

/* Sprite-stream argument of the overlay's icon grid at payload byte 0x8D34,
 * handed to func_801AF390 once between the equipment rows and the icon grid.
 * Only this body reads the run.
 * @source 0x801D9934 @kind data */
extern const u8 D_801D9934[];

/* Graphics-type query of the running executable, owned by the SDK map at
 * 0x8017B2B4. This payload only calls it. */
extern s32 GetGraphType(void);

/* Shaded-rectangle filler of the main executable: the two coordinates, the
 * width, the height, the shade byte and the CLUT-bank byte. This payload only
 * calls it.
 * @source 0x801AE3F0 */
extern void func_801AE3F0(u16 arg0, u16 arg1, u32 arg2, u32 arg3, u32 arg4,
                          u32 arg5);

/* Line-text drawer of the main executable: the two signed coordinates, the
 * shade byte and the string buffer it draws.
 * @source 0x80150098 */
extern void func_80150098(s16 arg0, s16 arg1, u32 arg2, const u8* arg3);

/* Panel-icon emitter of this overlay, defined by
 * emitPanelIconPrimShisu00_801D5620.c: the two zero-extended icon
 * coordinates, the icon kind, the size byte, the CLUT word and the palette
 * base byte. This payload only calls it.
 * @source 0x801D5620 */
extern void emitPanelIconPrimShisu00_801D5620(u16 arg0, u16 arg1, s32 arg2,
                                              s32 arg3, u16 arg4, u8 arg5);

/* Equipment preview builder of the main executable: the record index, the
 * caller argument this body forwards unchanged, and the two runs it fills, the
 * four status bytes and the four preview values. This payload only calls it.
 * @source 0x80164EF8 */
extern void func_80164EF8(u8 arg0, u8* arg1, u8* arg2, u16* arg3);

/* Palette cell emitter of the main executable: one cell at the two
 * zero-extended coordinates with the palette byte and the caller flag. This
 * payload only calls it.
 * @source 0x801AEA80 */
extern void func_801AEA80(u16 arg0, u16 arg1, u8 arg2, s32 arg3);

/* Sprite-mask query of the main executable: the low byte of the mask an
 * equipment object reports is the status byte of its row. This payload only
 * calls it.
 * @source 0x801AF20C */
extern u32 func_801AF20C(const void* arg0);

/* Sprite-stream submitter of the main executable: the two signed coordinates,
 * the stream row and the caller flag. This payload only calls it.
 * @source 0x801AF390 */
extern void func_801AF390(s16 arg0, s16 arg1, const u8* arg2, u8 arg3);

/* Single-icon emitter of the main executable: the two signed coordinates, the
 * sprite id out of the overlay icon set and the caller flag. This payload only
 * calls it.
 * @source 0x801AF2A0 */
extern void func_801AF2A0(s16 x, s16 y, u8 sprite_id, u8 flags);

/* @source 0x801D65DC
 * @behavior Party-member equipment and attribute panel of the SHISU overlay.
 *   The index byte selects one 164-byte field record out of D_80144968, and the
 *   body paints that member page in four parts. First the 0x78-by-0xA3 shaded
 *   rectangle at (x+4, y+4) through func_801AE3F0 with the shade 0 and the
 *   CLUT-bank byte D_80144952. Then the record name: the five leading glyph
 *   cells of the record are submitted through func_8014F800 at (x+0x20, y+7)
 *   as a five-cell run with alignment 0, four six-byte caption rows of
 *   D_801D9D18 follow at (x+5, y+0x1A/0x27/0x34/0x41) as four-cell runs, and
 *   four attribute halfwords of the record attribute block are formatted
 *   through sprintf into D_80145AD4 with the "%3d" string D_801D0C04 and drawn
 *   through func_80150098 at (x+0x31, y+0x1C/0x29/0x36/0x43) with the shade 0:
 *   in order the power halfword at record byte 0x20, the defense halfword at
 *   0x22, the intelligence halfword at 0x26 and the agility halfword at 0x24.
 *   Only when the caller flag byte is clear does the equipment grid follow:
 *   func_80164EF8 fills four status bytes and four preview values for the
 *   record index and the caller argument, and for each of those four rows a
 *   status byte picks one of the three icon kinds 0x11 for status 4, 0x12 for
 *   status 1 and 0x13 otherwise, a draw-mode packet of GetClut(0, 0x1E0) and
 *   the palette base 0x80 is installed through SetDrawMode and appended as
 *   0xC bytes after one graphics-type query, and the icon is emitted at
 *   (x+0x51, y+0x1C+13*row) through emitPanelIconPrimShisu00_801D5620 with
 *   size 0x1C beside that row preview value formatted as text at (x+0x59)
 *   with the status byte as shade. The six equipment slots then follow from
 *   the record ids at bytes 0x0E to 0x13 - the weapon record of
 *   WEAPON_OBJECTS, the armor records of ARMOR_OBJECTS and the accessory
 *   records of ACCESSORY_OBJECTS - as a palette cell at (x+6, row+2) with the
 *   per-slot byte of D_801D9D30 through func_801AEA80 and the object name
 *   through func_8014F800 at (x+0x11, row) with the low byte of the mask
 *   func_801AF20C reports for that object, where row is y plus 0x55 plus 13 per
 *   slot; the slot the record id byte 0x0B names takes the flag 1 on its
 *   palette cell, a second palette cell without the flag and a second name row
 *   two pixels higher with the count 2, and the slot the record id byte 0x0A
 *   names takes that same pair with the count 0. The body closes with the icon
 *   grid of the page: the D_801D9934 stream once at (x, y) through
 *   func_801AF390 with the flag 1, seven rows at (x, y+24+8*row) and
 *   (x+0x78, y+24+8*row) with the sprite ids 4 and 8, nine rows at
 *   (x, y+88+8*row) and (x+0x78, y+88+8*row) with the same pair, fourteen rows
 *   at (x+8+8*row, y+0x50) and (x+8+8*row, y+0xA0) with the ids 0x0E and 0x11,
 *   and the four corners (x, y+0x50) 0x0D, (x+0x78, y+0x50) 0x0F,
 *   (x, y+0xA0) 0x10 and (x+0x78, y+0xA0) 0x12, every one of them through
 *   func_801AF2A0 with the flag 1. No jal and no table word of this payload
 *   names this span, so a caller outside the payload runs it and the argument
 *   roles stay structural; the sixth argument is the record whose bytes 0x0A
 *   and 0x0B name the two armed equipment slots. Its 0xA0-byte frame keeps
 *   $s0 through $s7 and $fp beside $ra.
 * @status partial
 * @match 74.21
 * @residual 138 instructions differ, beginning at prologue offset 4; the
 *   candidate is 40 bytes shorter than the original. The original preserves
 *   raw coordinates beside the name/caption quantities, whereas this body
 *   shares their registers. Separate mutable coordinate phases, with and
 *   without explicit signed-halfword narrowing, measured 388/535 rather than
 *   this body's 397/535. The earlier 417/535 candidate read idx before its
 *   initialization; that invalid candidate is not a semantic baseline.
 */
void drawPartyMemberEquipAndAttributesPanelShisu00_801D65DC(u8 index, s32 x,
                                                            s32 y, u8* arg3,
                                                            u8 flag, u8* arg5) {
  u8 idx;
  u8* record;
  u8* attrs;
  u8 kinds[4];
  u16 values[4];
  const void* cells[6];
  u8 kind;
  u8 i;
  u8 flagCopy;
  u16 top;
  u32 mask;

  func_801AE3F0((u16)(x + 4), (u16)(y + 4), 0x78, 0xA3, 0, D_80144952[0]);
  idx = index;
  record = &D_80144952[0x16 + idx * 164];
  attrs = record + 0x1C;
  func_8014F800((s16)(x + 0x20), (s16)(y + 7), 0, 5, (u32)record);
  flagCopy = flag;
  top = y + 0x1A;
  for (i = 0; i < 4; i++) {
    func_8014F800((s16)(x + 5), (s16)(top + i * 13), 0, 4,
                  (u32)(D_801D9D18 + i * 6));
  }
  sprintf((char*)D_80145AD4, D_801D0C04, *(u16*)(attrs + 4));
  func_80150098((s16)(x + 0x31), (s16)(y + 0x1C), 0, D_80145AD4);
  sprintf((char*)D_80145AD4, D_801D0C04, *(u16*)(attrs + 6));
  func_80150098((s16)(x + 0x31), (s16)(y + 0x29), 0, D_80145AD4);
  sprintf((char*)D_80145AD4, D_801D0C04, *(u16*)(attrs + 0xA));
  func_80150098((s16)(x + 0x31), (s16)(y + 0x36), 0, D_80145AD4);
  sprintf((char*)D_80145AD4, D_801D0C04, *(u16*)(attrs + 8));
  func_80150098((s16)(x + 0x31), (s16)(y + 0x43), 0, D_80145AD4);

  if (flagCopy == 0) {
    func_80164EF8(idx, arg3, kinds, values);
    for (i = 0; i < 4; i++) {
      kind = kinds[i];
      if (kind == 4) {
        kind = 0x11;
      } else if (kind == 1) {
        kind = 0x12;
      } else {
        kind = 0x13;
      }
      if (GetGraphType() != 1) {
        GetGraphType();
      }
      SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, 0xF, 0);
      GpuAppendPrim(1, 0xC);
      emitPanelIconPrimShisu00_801D5620((u16)(x + 0x51),
                                        (u16)(top + i * 13 + 2),
                                        (s32)kind, 0x1C,
                                        (u16)GetClut(0, 0x1E0), 0x80);
      sprintf((char*)D_80145AD4, D_801D0C04, values[i]);
      func_80150098((s16)(x + 0x59), (s16)(top + i * 13 + 2), kinds[i],
                    D_80145AD4);
    }
  }

  cells[0] = &WEAPON_OBJECTS[record[0x0E]];
  cells[1] = &ARMOR_OBJECTS[record[0x0F]];
  cells[2] = &ARMOR_OBJECTS[record[0x10]];
  cells[3] = &ARMOR_OBJECTS[record[0x11]];
  cells[4] = &ACCESSORY_OBJECTS[record[0x12]];
  cells[5] = &ACCESSORY_OBJECTS[record[0x13]];
  for (i = 0; i < 6; i++) {
    if (i == arg5[0x0B]) {
      func_801AEA80((u16)(x + 6), (u16)(y + i * 13 + 0x57), D_801D9D30[i], 1);
      mask = func_801AF20C(cells[i]);
      func_8014F800((s16)(x + 0x11), (s16)(y + i * 13 + 0x55), 7, mask & 0xFF,
                    (u32)cells[i]);
      func_801AEA80((u16)(x + 6), (u16)(y + i * 13 + 0x55), D_801D9D30[i], 0);
      mask = func_801AF20C(cells[i]);
      func_8014F800((s16)(x + 0x11), (s16)(y + i * 13 + 0x53), 2, mask & 0xFF,
                    (u32)cells[i]);
    } else if (i == arg5[0x0A]) {
      func_801AEA80((u16)(x + 6), (u16)(y + i * 13 + 0x57), D_801D9D30[i], 1);
      mask = func_801AF20C(cells[i]);
      func_8014F800((s16)(x + 0x11), (s16)(y + i * 13 + 0x55), 7, mask & 0xFF,
                    (u32)cells[i]);
      func_801AEA80((u16)(x + 6), (u16)(y + i * 13 + 0x55), D_801D9D30[i], 0);
      mask = func_801AF20C(cells[i]);
      func_8014F800((s16)(x + 0x11), (s16)(y + i * 13 + 0x53), 0, mask & 0xFF,
                    (u32)cells[i]);
    } else {
      func_801AEA80((u16)(x + 6), (u16)(y + i * 13 + 0x57), D_801D9D30[i], 0);
      mask = func_801AF20C(cells[i]);
      func_8014F800((s16)(x + 0x11), (s16)(y + i * 13 + 0x55), 0, mask & 0xFF,
                    (u32)cells[i]);
    }
  }

  func_801AF390((s16)x, (s16)y, D_801D9934, 1);
  for (i = 0; i < 7; i++) {
    func_801AF2A0((s16)x, (s16)(y + (i + 3) * 8), 4, 1);
    func_801AF2A0((s16)(x + 0x78), (s16)(y + (i + 3) * 8), 8, 1);
  }
  for (i = 0; i < 9; i++) {
    func_801AF2A0((s16)x, (s16)(y + (i + 0xB) * 8), 4, 1);
    func_801AF2A0((s16)(x + 0x78), (s16)(y + (i + 0xB) * 8), 8, 1);
  }
  for (i = 0; i < 0xE; i++) {
    func_801AF2A0((s16)(x + (i + 1) * 8), (s16)(y + 0x50), 0xE, 1);
    func_801AF2A0((s16)(x + (i + 1) * 8), (s16)(y + 0xA0), 0x11, 1);
  }
  func_801AF2A0((s16)x, (s16)(y + 0x50), 0xD, 1);
  func_801AF2A0((s16)(x + 0x78), (s16)(y + 0x50), 0xF, 1);
  func_801AF2A0((s16)x, (s16)(y + 0xA0), 0x10, 1);
  func_801AF2A0((s16)(x + 0x78), (s16)(y + 0xA0), 0x12, 1);
}
