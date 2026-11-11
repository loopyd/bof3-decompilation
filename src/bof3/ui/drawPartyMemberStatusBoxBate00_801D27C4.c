#include "bof3/bof3.h"

/* Shared party-record table of the main executable. This body reads its byte 0
 * as the CLUT-bank argument of the inset painter and forms the selected record
 * 0x16 bytes past that byte, so the leading byte is the only cell named here.
 * @source 0x80144952 @kind data */
extern u8 D_80144952;

/* Shared sprintf work buffer of the main executable, outside this EMI payload:
 * every formatted value row is written here and then handed to the line-text
 * drawer as the buffer address.
 * @source 0x80145AD4 @kind data */
extern u8 D_80145AD4[];

/* Shared word of the main executable whose bit 0x20 selects the second status
 * marker when both status bits 0x80 and 0x20 are set.
 * @source 0x80143E6C @kind data */
extern s32 D_80143E6C;

/* Formats of this payload's leading data blob, at payload bytes 0x20, 0x24 and
 * 0x2C: the level row "%3d", the current value of a cur/max pair "%3d/   " and
 * the maximum value "   /%3d", the latter two drawn at one origin so they
 * overlay into a single row.
 * @source 0x801D0C20 @kind data */
extern const char D_801D0C20[];
/* @source 0x801D0C24 @kind data */
extern const char D_801D0C24[];
/* @source 0x801D0C2C @kind data */
extern const char D_801D0C2C[];

/* Sprite-record stream of this payload's trailing data blob, submitted at the
 * panel origin with the flag byte 0. Its bytes are zero on disc, like the
 * stream the sibling body in emi/etc/shisu/00 binds.
 * @source 0x801D8780 @kind data */
extern const u8 D_801D8780[];

/* Status-name strings of this payload's trailing data blob: the runs starting
 * "Pois" and "Conf" are the two status markers of the marker drawer.
 * @source 0x801D8B18 @kind data */
extern const u8 D_801D8B18[];
/* @source 0x801D8B20 @kind data */
extern const u8 D_801D8B20[];

/* Inset rectangle painter of the main executable: the two coordinates, the
 * width, the height, the shade and the CLUT-bank byte. Both coordinates arrive
 * zero-extended here, so those are the call-site widths.
 * @source 0x801AE3F0 */
extern void func_801AE3F0(u16 arg0, u16 arg1, u32 arg2, u32 arg3, u32 arg4,
                          u8 arg5);

/* Text submitter of the main executable: the pen origin, the alignment, the
 * cell count and the glyph-code row it queues.
 * @source 0x8014F800 */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* Line-text drawer of the main executable: the two signed coordinates, the
 * palette shade and the string it draws.
 * @source 0x80150098 */
extern void func_80150098(s16 arg0, s16 arg1, u32 arg2, const u8* arg3);

/* Status-marker drawer of the main executable: the two signed coordinates, the
 * shade, the width and the marker string.
 * @source 0x8014FC90 */
extern void func_8014FC90(s16 arg0, s16 arg1, s32 arg2, u32 arg3,
                          const u8* arg4);

/* Sprite-record stream emitter of the main executable: the two signed
 * coordinates, the stream blob and the flag byte it carries.
 * @source 0x801AF390 */
extern void func_801AF390(s16 arg0, s16 arg1, const u8* arg2, u8 arg3);

/* Portrait emitter of this overlay, still an `asm` row of this target's Splat
 * layout: the two zero-extended coordinates, the record index, the record level
 * byte and the record word at +8. Its call is the only fact this body binds.
 * @source 0x801D4730 */
extern void func_801D4730(u16 arg0, u16 arg1, u32 arg2, u32 arg3, u32 arg4);

/* Icon-sprite emitter of this overlay at 0x801D3664, already lifted exact in
 * this target: the original zero-extends both coordinates before this call, so
 * this translation unit declares the call-site widths. */
void drawSpriteWithScenarioKindSubstitutionBate00_801D3664(u16 x, u16 y,
                                                           u8 kind, u8 mode);

extern int sprintf(char* buffer, const char* format, ...);

/* Word/halfword views of the 0x0C status pair of the 164-byte party records at
 * 0x80144968: the two-marker test reads the whole word, the marker and the
 * low-value bit tests read the halfword. */
typedef union BatePartyStatus {
  u32 word;
  u16 half;
} BatePartyStatus;

/* One 164-byte entry of the shared party-record table selected by the index
 * byte; only the fields this body consumes are named. */
typedef struct BatePartyStatusRecord {
  u8 name[5];            /* +0x00 five glyph codes of the name run */
  u8 icon;               /* +0x05 icon kind handed to the sprite emitter */
  u8 level;              /* +0x06 level byte formatted "%3d" */
  u8 unk_07;             /* +0x07 */
  u32 unk_08;            /* +0x08 word handed to the portrait emitter */
  BatePartyStatus status; /* +0x0C */
  u8 unk_10[4];          /* +0x10 */
  u16 hp;                /* +0x14 current HP */
  u16 mp;                /* +0x16 current MP */
  u8 unk_18[2];          /* +0x18 */
  u8 marker;             /* +0x1A nonzero brightens the maximum HP row */
  u8 unk_1B;             /* +0x1B */
  u16 hpLimit;           /* +0x1C maximum HP, quartered by the low-HP test */
  u16 mpLimit;           /* +0x1E maximum MP, quartered by the low-MP test */
  u8 unk_20[0x84];       /* record tail, stride is 0xA4 */
} BatePartyStatusRecord;

/* @source 0x801D27C4
 * @behavior Party-member status box of the BATE overlay: it fills the 0x7D by
 *   0x30 inset rectangle at (x+3, y+3) through func_801AE3F0 with the panel
 *   flag as its shade and the CLUT-bank byte D_80144952[0], then selects the
 *   164-byte party record at 0x80144968 - the byte 0x16 past the CLUT cell - by
 *   its index byte and draws that member: the record icon at
 *   (x+0x56, y+2) through drawSpriteWithScenarioKindSubstitutionBate00_801D3664,
 *   whose mode is 1 for a flagged box and otherwise bit 1 of the status word
 *   shifted down by six; the five name glyph codes of the record head through
 *   func_8014F800 at (x+0x14, y+3) with that shade as its alignment; the level
 *   byte through sprintf with "%3d"
 *   at (x+0x16, y+0x17); the status marker through func_8014FC90 at
 *   (x+0x2E, y+0x17), which is the "Pois" run 0x801D8B18 when both status bits
 *   0x80 and 0x20 are set and the shared word 0x80143E6C carries bit 0x20 or
 *   when bit 0x80 alone is set, the "Conf" run 0x801D8B20 when that shared word
 *   is clear or when bit 0x20 alone is set, and nothing when none of those
 *   hold; the two cur/max rows of HP ("%3d/   " then "   /%3d" at y+0x1F) and
 *   of MP (the same pair at y+0x27), each formatted from the record halfwords
 *   0x14 and 0x1C and 0x16 and 0x1E and drawn with the row shade 7 for a
 *   flagged box and otherwise 4 or 2 or 0. Before the HP rows it rewrites
 *   status bit 0x2000 from the low-HP test - set when the current HP is below a
 *   quarter of the maximum and cleared otherwise - and it takes the 4 shade
 *   from that stored bit while the 2 shade comes from a current HP below two
 *   and, on the MP row, from a below-a-quarter maximum and from a current MP of
 *   zero. It closes with the sprite stream D_801D8780 submitted at the panel
 *   origin with the flag byte 0 and the portrait func_801D4730 at
 *   (x+0x14, y+0x12) for the record index, its level byte and its word at +8.
 *   No jal and no word of any extracted payload names this span, so it is an
 *   overlay entry called from outside the payload and its four arguments have
 *   the roles their uses give them. Its 0x48-byte frame keeps $s0 through $s7
 *   and $fp beside $ra, spills the index byte to 0x18($sp) across the first
 *   call, and the epilogue returns no value. A payload word scan proved the
 *   0x468 bytes equal drawPartyMemberStatusBoxShisu00_801D376C at 0x801D376C in
 *   emi/etc/shisu/00, an exact lift there, across all 282 instruction words and
 *   every register field, differing only in 18 words: the six internal jump
 *   words and the two jal targets, each displaced by the 0xFA8 delta between
 *   the two spans, and the eight words of this overlay's own carried constants
 *   - the three format cells, the two marker strings and the sprite stream -
 *   which are read from this payload; the shop/00 copy of the same body is
 *   shape evidence only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawPartyMemberStatusBoxBate00_801D27C4(u16 x, u16 y, u8 index, u8 flag) {
  u8* base;
  BatePartyStatusRecord* record;
  u8 align;
  s16 textX;
  s16 textY;

  align = flag ? 7 : 0;
  base = &D_80144952;
  func_801AE3F0((u16)(x + 3), (u16)(y + 3), 0x7D, 0x30, flag, base[0]);
  record = (BatePartyStatusRecord*)(&D_80144952 + 0x16) + index;
  drawSpriteWithScenarioKindSubstitutionBate00_801D3664(
      (u16)(x + 0x56), (u16)(y + 2), record->icon,
      flag ? 1 : (u8)((record->status.half >> 6) & 2));
  func_8014F800((s16)(x + 0x14), (s16)(y + 3), align, 5, (u32)record);
  sprintf((char*)D_80145AD4, D_801D0C20, record->level);
  func_80150098((s16)(x + 0x16), (s16)(y + 0x17), align, D_80145AD4);

  if ((record->status.word & 0xA0) == 0xA0) {
    if (D_80143E6C & 0x20) {
      func_8014FC90((s16)(x + 0x2E), (s16)(y + 0x17), align ? align : 1, 0x10,
                    D_801D8B18);
    } else {
      func_8014FC90((s16)(x + 0x2E), (s16)(y + 0x17), align ? align : 1, 0x10,
                    D_801D8B20);
    }
  } else if (record->status.half & 0x80) {
    func_8014FC90((s16)(x + 0x2E), (s16)(y + 0x17), align ? align : 1, 0x10,
                  D_801D8B18);
  } else if (record->status.half & 0x20) {
    func_8014FC90((s16)(x + 0x2E), (s16)(y + 0x17), align ? align : 1, 0x10,
                  D_801D8B20);
  }

  if (record->hp < (record->hpLimit >> 2)) {
    record->status.half |= 0x2000;
  } else {
    record->status.half &= 0xDFFF;
  }

  if (!flag) {
    if (record->status.half & 0x2000) {
      align = 4;
    }
    if (record->hp < 2) {
      align = 2;
    }
  } else {
    align = 7;
  }
  sprintf((char*)D_80145AD4, D_801D0C24, record->hp);
  func_80150098((s16)(x + 0x16), (s16)(y + 0x1F), align, D_80145AD4);

  if (!flag) {
    align = record->marker ? 4 : 0;
  } else {
    align = 7;
  }
  sprintf((char*)D_80145AD4, D_801D0C2C, record->hpLimit);
  func_80150098((s16)(x + 0x16), (s16)(y + 0x1F), align, D_80145AD4);

  align = flag ? 7 : 0;
  if (!flag) {
    if (record->mp < (record->mpLimit >> 2)) {
      align = 4;
    }
    if (record->mp == 0) {
      align = 2;
    }
  } else {
    align = 7;
  }
  sprintf((char*)D_80145AD4, D_801D0C24, record->mp);
  textX = (s16)(x + 0x16);
  textY = (s16)(y + 0x27);
  func_80150098(textX, textY, align, D_80145AD4);

  align = flag ? 7 : 0;
  sprintf((char*)D_80145AD4, D_801D0C2C, record->mpLimit);
  func_80150098(textX, textY, align, D_80145AD4);

  func_801AF390((s16)x, (s16)y, D_801D8780, 0);
  func_801D4730((u16)(x + 0x14), (u16)(y + 0x12), index, record->level,
                record->unk_08);
}
