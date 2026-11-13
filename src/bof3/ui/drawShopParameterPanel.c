#include "bof3/ui/shop00_internal.h"

/* Decimal format string of this EMI's payload header blob, at payload byte
 * 0xE4: the three-character field "%3d" every value row of this panel formats
 * its halfword with. The address sits in the leading header blob, so its
 * generated undefined-symbol row carries it and no target-map row is added.
 * @source 0x801D0CE4 @kind rodata */
extern const char D_801D0CE4[];

/* Caption cells of this EMI's payload, at payload byte 0x14E10: three
 * six-byte cells holding "Pwr", "Def" and "Agl", the labels of the three value
 * rows this panel draws on its left edge. The body takes each cell address as
 * the fifth argument of func_8014F800, so the cells are read only through that
 * pointer and their role is "the caption this panel submits"; the name stays
 * the generated one.
 * @source 0x801E5A10 @kind data */
extern const u8 D_801E5A10[];

/* Sprite-record stream of this EMI's payload, at payload byte 0x14ADC: a run
 * of 3-byte records submitted once at the panel origin through func_801AF390
 * with the flag byte 0. Nothing in the payload interprets the record fields,
 * so the blob keeps the generated name.
 * @source 0x801E56DC @kind data */
extern const u8 D_801E56DC[];

/* Caption drawer of the main executable: the two signed coordinates, the
 * palette shade this body derives per row, the 0xFF mask and the caption cell
 * address, which the original stores to the outgoing stack slot.
 * @source 0x8014F800 */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* Sprite-record stream emitter of the main executable: the two signed
 * coordinates, the stream blob and the flag byte it carries.
 * @source 0x801AF390 */
extern void func_801AF390(s16 arg0, s16 arg1, const u8* arg2, u8 arg3);

/* Shop icon wrapper of this EMI (src/bof3/ui/func_801D910C_shop00.c), lifted
 * exact: it substitutes the icon kind 0xB for the kind 4 in one save-state
 * case and forwards the four arguments to emitShopIconSprite. Here it receives
 * the two zero-extended coordinates, the record's id byte as the icon kind and
 * the panel flag as the mode byte.
 * @source 0x801D910C */
extern void func_801D910C(u16 arg0, u16 arg1, u8 arg2, u8 arg3);

/* Group query of the main executable: two bytes in, one byte out, of which
 * this body reads the single bit its record index selects. The address has no
 * prototype in this target before this lift; it is bound by the generated
 * undefined-symbol list, so no target-map row is added.
 * @source 0x80165EC0 */
extern u8 func_80165EC0(u8 first, u8 second);

/* Value preview builder of the main executable: the record index, the
 * zero-based mode byte and the second task byte, then the four-cell shade run
 * and the four-cell value run it fills for the flag-clear arm of this panel.
 * No source of this target calls it with any other argument set, so the widths
 * below are the widths observed here, not a recovered callee contract.
 * @source 0x80164D2C */
extern void func_80164D2C(u8 arg0, u8 arg1, u8 arg2, u8* arg3, u16* arg4);

extern int sprintf(char* buffer, const char* format, ...);

/* One 164-byte entry of the shared field record table D_80144968 selected by
 * the record index; only the fields this body reads are named. The record id
 * at +5 is the value D_80181B10 maps a list entry back to and the byte this
 * panel hands to the icon wrapper, and the three attribute halfwords are the
 * values formatted beside the "Pwr", "Def" and "Agl" captions.
 * @source 0x80144968 @kind table */
typedef struct ShopParameterPanelRecord {
  u8 unk_00[5];
  u8 id;       /* +0x05 record id, drawn as the portrait icon kind */
  u8 unk_06[0x1A];
  u16 power;   /* +0x20 formatted "%3d" beside the "Pwr" caption */
  u16 defense; /* +0x22 beside "Def" */
  u16 agility; /* +0x24 beside "Agl" */
  u8 unk_26[0x7E];
} ShopParameterPanelRecord;

/* @source 0x801DB9F0
 * @behavior shop parameter panel of the field record: groups the task mode
 *   byte arg3 with the 13-slot run that sends 0, 6, 7, 8, 9, 12 and every
 *   larger value to group 0, 1 to group 1, 2, 3, 4 to group 2 and 5, 10, 11 to
 *   group 3, asks the main-exe group query func_80165EC0 for that group with
 *   the task byte arg4 and takes bit arg2 of the answer inverted as the panel
 *   flag. It then fills the 0x6C by 0x30 shaded rectangle at (x+3, y+3)
 *   through func_801AE3F0 with the CLUT-bank byte D_80144952. With the flag
 *   clear it hands the record index, arg3 minus one and arg4 to the main-exe
 *   preview builder func_80164D2C, whose four-cell value run has its fourth
 *   cell copied over the third, and formats the three values that remain with
 *   "%3d" into the shared buffer D_80145AD4 and draws each through
 *   func_80150098 at (x+0x2C, y + 15*i + 8) with that row's own shade byte.
 *   With the flag set it formats the record's own power, defense and agility
 *   halfwords at +0x20, +0x22 and +0x24 the same way and draws them in the
 *   same column with the shade 7. It then draws the record id byte through the
 *   shop icon wrapper func_801D910C at (x+0x46, y+2), submits the three
 *   caption cells of D_801E5A10 at (x+5, y + 15*i + 6) through func_8014F800
 *   with the mask 0xFF and the shade the value rows used, and closes with the
 *   stream D_801E56DC at (x, y) through func_801AF390 with the flag 0. Nothing
 *   is returned and the 0x58-byte frame keeps $s0 through $s7 and $fp beside
 *   $ra across the calls, spilling the flag byte to 0x28($sp). The compact
 *   form of the flag is exact: the group answer is a byte, so the inverted bit
 *   is masked to 0x1 before it is spilled, and the preview's fourth value and
 *   fourth shade replace the third slot of their runs before the three drawn
 *   rows read them. No jal and no word of this payload names the span, so its
 *   five arguments have the roles their uses give them: the dispatcher
 *   func_801E2C18 invokes it from the panel task and the widths are that call
 *   site's.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawShopParameterPanel(u16 x, u16 y, u8 index, u8 mode, u8 arg4) {
  ShopParameterPanelRecord* record;
  u8 shades[4];
  u16 values[4];
  u8 shade = 0;
  u8 kind;
  u8 flag;
  u8 i;

  record = (ShopParameterPanelRecord*)(D_80144968 + index * 164);

  /* The group 0 arm carries the out-of-range default as well, so the compiler
   * guards the run with one unsigned bound and shares that arm's body. */
  switch (mode) {
  case 0:
  case 6:
  case 7:
  case 8:
  case 9:
  case 12:
  default:
    kind = 0;
    break;
  case 1:
    kind = 1;
    break;
  case 2:
  case 3:
  case 4:
    kind = 2;
    break;
  case 5:
  case 10:
  case 11:
    kind = 3;
    break;
  }

  flag = ((func_80165EC0(kind, arg4) >> index) ^ 1) & 1;
  func_801AE3F0((u16)(x + 3), (u16)(y + 3), 0x6C, 0x30, flag, D_80144952);

  if (flag == 0) {
    func_80164D2C(index, (u8)(mode - 1), arg4, shades, values);
    values[2] = values[3];
    shades[2] = shades[3];
    for (i = 0; i < 3; i++) {
      sprintf((char*)D_80145AD4, D_801D0CE4, values[i]);
      func_80150098((s16)(x + 0x2C), (s16)(y + i * 15 + 8), shades[i],
                    D_80145AD4);
    }
  } else {
    shade = 7;
    sprintf((char*)D_80145AD4, D_801D0CE4, record->power);
    func_80150098((s16)(x + 0x2C), (s16)(y + 8), 7, D_80145AD4);
    sprintf((char*)D_80145AD4, D_801D0CE4, record->defense);
    func_80150098((s16)(x + 0x2C), (s16)(y + 0x17), 7, D_80145AD4);
    sprintf((char*)D_80145AD4, D_801D0CE4, record->agility);
    func_80150098((s16)(x + 0x2C), (s16)(y + 0x26), 7, D_80145AD4);
  }

  func_801D910C((u16)(x + 0x46), (u16)(y + 2), record->id, flag);

  for (i = 0; i < 3; i++) {
    func_8014F800((s16)(x + 5), (s16)(y + i * 15 + 6), shade, 0xFF,
                  (u32)(D_801E5A10 + i * 6));
  }

  func_801AF390((s16)x, (s16)y, D_801E56DC, 0);
}
