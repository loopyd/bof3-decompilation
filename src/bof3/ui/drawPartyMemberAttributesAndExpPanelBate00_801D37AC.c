#include "bof3/bof3.h"

/* Shared party-record table of the main executable, viewed at the attribute
 * block of the 164-byte record: the index byte of this panel selects one
 * record, and only the four attribute halfwords this body formats are named.
 * The generated undefined_syms_auto.txt of this target already carries this
 * address, so no local map row is added here.
 * @source 0x80144984 @kind table */
extern u8 D_80144984[];

/* Interior bytes of the same 164-byte party record: the level byte at record
 * byte 0x06, which this panel tests against the level cap, and the experience
 * word at record byte 0x08, which it formats into the experience row.
 * @source 0x8014496E @kind data */
extern u8 D_8014496E[];
/* @source 0x80144970 @kind data */
extern u8 D_80144970[];

/* Shared sprintf work buffer of the main executable, outside this EMI payload:
 * every formatted value row is written here and then handed to the line-text
 * drawer as the buffer address.
 * @source 0x80145AD4 @kind data */
extern u8 D_80145AD4[];

/* Formats and placeholder of this overlay's leading data blob, at payload bytes
 * 0x20, 0x44 and 0x48: the attribute value format "%3d", the experience row
 * format "%7d" and the seven-dash placeholder that replaces it at the level
 * cap.
 * @source 0x801D0C20 @kind data */
extern const char D_801D0C20[];
/* @source 0x801D0C44 @kind data */
extern const char D_801D0C44[];
/* @source 0x801D0C48 @kind data */
extern const char D_801D0C48[];

/* Caption run of this overlay's trailing data blob, at payload byte 0x7F84:
 * four six-byte rows starting "Pwr", "Def", "Int" and "Agl", each submitted as
 * a four-cell run at one corner of the attribute grid.
 * @source 0x801D8B84 @kind data */
extern const u8 D_801D8B84[];

/* Inset rectangle painter of the main executable: it fills the width by height
 * area at the two coordinates with the shade its fifth argument carries. This
 * payload only calls it.
 * @source 0x801AED54 */
extern void func_801AED54(s32 x, s32 y, s32 width, s32 height, s32 shade);

/* Line-text drawer of the main executable: the two signed coordinates, the
 * alignment argument and the string buffer it draws. This payload only calls
 * it.
 * @source 0x80150098 */
extern void func_80150098(s16 arg0, s16 arg1, u32 arg2, const u8* arg3);

/* Glyph-run submitter of the main executable: the two signed coordinates, the
 * alignment, the cell count and the address of the glyph-code row it queues.
 * @source 0x8014F800 */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* Sprite emitter of this overlay, defined by
 * drawSpritePrimTpageFromGraphTypeBate00_801D4564.c: the icon kind at the two
 * zero-extended coordinates with the flag byte. The original zero-extends both
 * coordinates before these calls, so this translation unit declares the
 * call-site widths. */
void drawSpritePrimTpageFromGraphTypeBate00_801D4564(u16 arg0, u16 arg1,
                                                     u8 arg2, u8 arg3);

/* Experience-table sum of this overlay, defined by sumLevelExpBate00_801D4FD0.c:
 * the character row and the number of level entries it adds. */
extern s32 sumLevelExpBate00_801D4FD0(u8 row, u8 count);

extern int sprintf(char* buffer, const char* format, ...);

/* One 164-byte party record of the shared main-executable table, viewed at the
 * attribute block this panel formats; only the fields this body consumes are
 * named. */
typedef struct BateAttributeBlock {
  u8 unk_00[4];     /* +0x00 */
  u16 power;        /* +0x04 formatted "%3d" beside the "Pwr" caption */
  u16 defense;      /* +0x06 beside "Def" */
  u16 agility;      /* +0x08 beside "Agl" */
  u16 intelligence; /* +0x0A beside "Int" */
  u8 unk_0C[0x98];  /* record tail, the record stride is 0xA4 */
} BateAttributeBlock;

/* @source 0x801D37AC
 * @behavior Party-member attribute and experience panel of the BATE overlay:
 *   it paints the 0x86 by 0x2A inset rectangle at (x+2, y+2) through
 *   func_801AED54 with the shade 0xF1 and then fills a two-by-two attribute
 *   grid of the 164-byte party record the index byte selects out of D_80144984.
 *   Each attribute halfword is formatted with "%3d" into the shared buffer
 *   D_80145AD4 through sprintf and drawn through func_80150098 at (x+0x29, y+5)
 *   for the record word at +0x04, (x+0x29, y+0x12) for +0x06, (x+0x6F, y+5) for
 *   +0x0A and (x+0x6F, y+0x12) for +0x08, and the four six-byte caption rows of
 *   D_801D8B84 - "Pwr", "Def", "Int" and "Agl" - follow as four-cell runs
 *   through func_8014F800 at (x+5, y+3), (x+5, y+0x10), (x+0x4B, y+3) and
 *   (x+0x4B, y+0x10). It then paints the 0x56 by 0x14 inset rectangle at
 *   (x+0x33, y+0x2E) with the shade 0xF1 and fills the two value rows inside
 *   it: the experience word at record byte 0x08 formatted "%7d" at
 *   (x+0x4E, y+0x30), and the experience total of the next level at
 *   (x+0x4E, y+0x39), which sumLevelExpBate00_801D4FD0 forms over the
 *   record level byte 0x06 plus one and which the placeholder "-------"
 *   replaces once that level byte reaches 0x63. Each value row is preceded by
 *   drawSpritePrimTpageFromGraphTypeBate00_801D4564 at (x+0x36, y+0x30) with
 *   kind 4 and at (x+0x36, y+0x39) with kind 5. The alignment argument of the
 *   text and glyph calls is 7 when the flag byte is nonzero and 0 when it is
 *   zero. Nothing is returned and the 0x58-byte frame keeps $s0 through $s7
 *   and $fp beside $ra, spilling the index byte to 0x28($sp), the flag byte to
 *   0x20($sp) and the alignment byte to 0x18($sp) across the calls.
 *   The payload names this span from the jal at 0x801D5F28 inside
 *   func_801D5E90, which passes the task origin halfwords, the leading byte of
 *   the 0x140-byte record D_80145FCC[task byte 0xC] and the low byte of its own
 *   second argument, right after the equipment-slot block
 *   drawEquipSlotRowsBate00_801D3B4C for the same arguments. One payload cmp
 *   proved these 0x3A0 bytes equal drawPartyMemberAttributesAndExpPanelShisu00_801D4754
 *   at 0x801D4754 in emi/etc/shisu/00 and func_801D9254 at 0x801D9254 in
 *   emi/etc/shop/00 apart from eight words - the low halves of the two payload
 *   caption and format addresses, the three jal target words and the forward
 *   jump word - while every lui half stayed equal; both copies are still asm
 *   rows there.
 * @status partial
 * @match 96.60
 * @residual the row origin of the experience rows. These 0x3A0 bytes are
 *   byte-exact through the level load and its 0x63 test, and the three extra
 *   instructions sit below that test: the original fills the row origin with
 *   one plain register copy (move s0,s8) in the beqz delay slot and computes
 *   the capped row origin for free by reusing the y+0x30 word the preceding
 *   value row already holds, while this body declares the two coordinates s16
 *   and the row origin s32, so the mode pass sign-extends y at the copy and
 *   rebuilds the +0x30 on the capped arm, three instructions the original does
 *   not carry. Declaring y full width removes all three and restores the 928
 *   original bytes, but the two coordinate copies at 0x801D3A0C..0x801D3A1C
 *   then swap the $s0 and $s1 saved registers, and that swap held through the
 *   same copy with the assignment order or the branch polarity changed and
 *   through a full-width row origin local, which additionally cost a tenth
 *   saved register.
 */
void drawPartyMemberAttributesAndExpPanelBate00_801D37AC(s16 x, s16 y,
                                                         u8 index, u8 flag) {
  BateAttributeBlock* record;
  u8 align;
  u8 level;
  s32 rowY;

  align = flag ? 7 : 0;
  func_801AED54((s16)(x + 2), (s16)(y + 2), 0x86, 0x2A, 0xF1);
  record = (BateAttributeBlock*)&D_80144984[index * 164];
  sprintf((char*)D_80145AD4, D_801D0C20, record->power);
  func_80150098((s16)(x + 0x29), (s16)(y + 5), align, D_80145AD4);
  sprintf((char*)D_80145AD4, D_801D0C20, record->defense);
  func_80150098((s16)(x + 0x29), (s16)(y + 0x12), align, D_80145AD4);
  sprintf((char*)D_80145AD4, D_801D0C20, record->intelligence);
  func_80150098((s16)(x + 0x6F), (s16)(y + 5), align, D_80145AD4);
  sprintf((char*)D_80145AD4, D_801D0C20, record->agility);
  func_80150098((s16)(x + 0x6F), (s16)(y + 0x12), align, D_80145AD4);
  func_8014F800((s16)(x + 5), (s16)(y + 3), align, 4, (u32)D_801D8B84);
  func_8014F800((s16)(x + 5), (s16)(y + 0x10), align, 4,
                (u32)(D_801D8B84 + 6));
  func_8014F800((s16)(x + 0x4B), (s16)(y + 3), align, 4,
                (u32)(D_801D8B84 + 0xC));
  func_8014F800((s16)(x + 0x4B), (s16)(y + 0x10), align, 4,
                (u32)(D_801D8B84 + 0x12));
  func_801AED54((s16)(x + 0x33), (s16)(y + 0x2E), 0x56, 0x14, 0xF1);
  drawSpritePrimTpageFromGraphTypeBate00_801D4564((u16)(x + 0x36),
                                                  (u16)(y + 0x30), 4, flag);
  sprintf((char*)D_80145AD4, D_801D0C44, *(s32*)&D_80144970[index * 164]);
  func_80150098((s16)(x + 0x4E), (s16)(y + 0x30), align, D_80145AD4);
  drawSpritePrimTpageFromGraphTypeBate00_801D4564((u16)(x + 0x36),
                                                  (u16)(y + 0x39), 5, flag);
  level = D_8014496E[index * 164];
  if (level < 0x63) {
    sprintf((char*)D_80145AD4, D_801D0C44,
            sumLevelExpBate00_801D4FD0(index, (u8)(level + 1)));
    rowY = y;
  } else {
    sprintf((char*)D_80145AD4, D_801D0C48);
    rowY = y + 0x30;
  }
  func_80150098((s16)(x + 0x4E), (s16)(rowY + 0x39), align, D_80145AD4);
}
