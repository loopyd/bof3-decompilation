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

/* Formats of this overlay's leading data blob, at payload bytes 4, 0x28 and
 * 0x2C: the attribute value format "%3d", the experience row format "%7d" and
 * the eight-dash placeholder that replaces it at the level cap.
 * @source 0x801D0C04 @kind data */
extern const char D_801D0C04[];
/* @source 0x801D0C28 @kind data */
extern const char D_801D0C28[];
/* @source 0x801D0C2C @kind data */
extern const char D_801D0C2C[];

/* Caption run of this overlay's trailing data blob, at payload byte 0x8FA8:
 * four six-byte rows starting "Pwr", "Def", "Int" and "Agl", each submitted as
 * a four-cell run at one corner of the attribute grid.
 * @source 0x801D9BA8 @kind data */
extern const u8 D_801D9BA8[];

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
 * drawSpritePrimTpageFromGraphTypeShisu00_801D550C.c: the icon kind at the two
 * zero-extended coordinates with the flag byte. The original zero-extends both
 * coordinates before these calls, so this translation unit declares the
 * call-site widths. */
void drawSpritePrimTpageFromGraphTypeShisu00_801D550C(u16 arg0, u16 arg1,
                                                      u8 arg2, u8 arg3);

/* Experience-table sum of this overlay, defined by
 * sumLevelExpShisu00_801D5F78.c: the character row and the number of level
 * entries it adds. */
extern s32 sumLevelExpShisu00_801D5F78(u8 row, u8 count);

extern int sprintf(char* buffer, const char* format, ...);

/* One 164-byte party record of the shared main-executable table, viewed at the
 * attribute block this panel formats; only the fields this body consumes are
 * named. */
typedef struct ShisuAttributeBlock {
  u8 unk_00[4];     /* +0x00 */
  u16 power;        /* +0x04 formatted "%3d" beside the "Pwr" caption */
  u16 defense;      /* +0x06 beside "Def" */
  u16 agility;      /* +0x08 beside "Agl" */
  u16 intelligence; /* +0x0A beside "Int" */
  u8 unk_0C[0x98];  /* record tail, the record stride is 0xA4 */
} ShisuAttributeBlock;

/* @source 0x801D4754
 * @behavior Party-member attribute and experience panel of the SHISU overlay:
 *   it paints the 0x86 by 0x2A inset rectangle at (x+2, y+2) through
 *   func_801AED54 with the shade 0xF1 and then fills a two-by-two attribute
 *   grid of the 164-byte party record the index byte selects out of D_80144984.
 *   Each attribute halfword is formatted with "%3d" into the shared buffer
 *   D_80145AD4 through sprintf and drawn through func_80150098 at (x+0x29, y+5)
 *   for the record word at +0x04, (x+0x29, y+0x12) for +0x06, (x+0x6F, y+5) for
 *   +0x0A and (x+0x6F, y+0x12) for +0x08, and the four six-byte caption rows of
 *   D_801D9BA8 - "Pwr", "Def", "Int" and "Agl" - follow as four-cell runs
 *   through func_8014F800 at (x+5, y+3), (x+5, y+0x10), (x+0x4B, y+3) and
 *   (x+0x4B, y+0x10). It then paints the 0x56 by 0x14 inset rectangle at
 *   (x+0x33, y+0x2E) with the shade 0xF1 and fills the two value rows inside
 *   it: the experience word at record byte 0x08 formatted "%7d" at
 *   (x+0x4E, y+0x30), and the experience total of the next level at
 *   (x+0x4E, y+0x39), which sumLevelExpShisu00_801D5F78 forms over the
 *   record level byte 0x06 plus one and which the placeholder "-------"
 *   replaces once that level byte reaches 0x63. Each value row is preceded by
 *   drawSpritePrimTpageFromGraphTypeShisu00_801D550C at (x+0x36, y+0x30) with
 *   kind 4 and at (x+0x36, y+0x39) with kind 5. The alignment argument of the
 *   text and glyph calls is 7 when the flag byte is nonzero and 0 when it is
 *   zero. Nothing is returned and the 0x58-byte frame keeps $s0 through $s7
 *   and $fp beside $ra, spilling the index byte to 0x28($sp), the flag byte to
 *   0x20($sp) and the alignment byte to 0x18($sp) across the calls.
 *   The payload names this span from the jal at 0x801D6ED0 inside
 *   func_801D6E38, which passes the task origin halfwords, the leading byte of
 *   the 0x140-byte record D_80145FCC[task byte 0xC] and the low byte of its own
 *   second argument, right after the equipment-slot block
 *   drawEquipSlotRowsShisu00_801D4AF4 for the same arguments. One payload cmp
 *   proved these 0x3A0 bytes equal func_801D37AC at 0x801D37AC in
 *   emi/etc/bate/00 and func_801D9254 at 0x801D9254 in emi/etc/shop/00 apart
 *   from eight words - the low halves of the two payload caption and format
 *   addresses, the three jal target words and the forward jump word - while
 *   every lui half stayed equal; both copies are still asm rows there, so no
 *   sibling source carried this body.
 * @status partial
 * @match 96.60
 * @residual the level-cap tail. These 0x3A0 bytes are byte-exact through the
 *   level load and its 0x63 test, and the two hunks that remain sit in the
 *   arms below that test. The original copies y into the saved register that
 *   already holds the experience row origin (move s0,s8 in the beqz delay
 *   slot) and leaves that register untouched on the level-capped arm, whose
 *   placeholder sprintf emits no store at all, so the shared text call adds
 *   0x39 to that register and the capped arm reaches it holding the shifted
 *   value. This body declares the row origin s32 and both coordinates s16, and
 *   the mode pass shortens that local to the narrow form: it converts y at the
 *   copy (sll v0,s8,0x10; sra v0,v0,0x10), rebuilds the +0x30 on the capped
 *   arm and adds 0x39 to that scratch register, three instructions the original
 *   does not carry. Declaring y full width removes all three and reproduces the
 *   capped arm's empty body, but the two coordinate copies at
 *   0x801D49B4..0x801D49C4 then swap the s0 and s1 saved registers, the taken
 *   arm's copy leaves the delay slot and the tail adds 0x39 to s1. Fourteen
 *   further measured variants of the tail shape, the local's width, its
 *   declaration order, an early-armed copy and a mutated parameter kept that
 *   swap, so the remaining difference reads as psi's register order for that
 *   copy group and not as a statement-order choice.
 */
void drawPartyMemberAttributesAndExpPanelShisu00_801D4754(s16 x, s16 y,
                                                          u8 index, u8 flag) {
  ShisuAttributeBlock* record;
  u8 align;
  u8 level;
  s32 rowY;

  align = flag ? 7 : 0;
  func_801AED54((s16)(x + 2), (s16)(y + 2), 0x86, 0x2A, 0xF1);
  record = (ShisuAttributeBlock*)&D_80144984[index * 164];
  sprintf((char*)D_80145AD4, D_801D0C04, record->power);
  func_80150098((s16)(x + 0x29), (s16)(y + 5), align, D_80145AD4);
  sprintf((char*)D_80145AD4, D_801D0C04, record->defense);
  func_80150098((s16)(x + 0x29), (s16)(y + 0x12), align, D_80145AD4);
  sprintf((char*)D_80145AD4, D_801D0C04, record->intelligence);
  func_80150098((s16)(x + 0x6F), (s16)(y + 5), align, D_80145AD4);
  sprintf((char*)D_80145AD4, D_801D0C04, record->agility);
  func_80150098((s16)(x + 0x6F), (s16)(y + 0x12), align, D_80145AD4);
  func_8014F800((s16)(x + 5), (s16)(y + 3), align, 4, (u32)D_801D9BA8);
  func_8014F800((s16)(x + 5), (s16)(y + 0x10), align, 4,
                (u32)(D_801D9BA8 + 6));
  func_8014F800((s16)(x + 0x4B), (s16)(y + 3), align, 4,
                (u32)(D_801D9BA8 + 0xC));
  func_8014F800((s16)(x + 0x4B), (s16)(y + 0x10), align, 4,
                (u32)(D_801D9BA8 + 0x12));
  func_801AED54((s16)(x + 0x33), (s16)(y + 0x2E), 0x56, 0x14, 0xF1);
  drawSpritePrimTpageFromGraphTypeShisu00_801D550C((u16)(x + 0x36),
                                                   (u16)(y + 0x30), 4, flag);
  sprintf((char*)D_80145AD4, D_801D0C28, *(s32*)&D_80144970[index * 164]);
  func_80150098((s16)(x + 0x4E), (s16)(y + 0x30), align, D_80145AD4);
  drawSpritePrimTpageFromGraphTypeShisu00_801D550C((u16)(x + 0x36),
                                                   (u16)(y + 0x39), 5, flag);
  level = D_8014496E[index * 164];
  if (level < 0x63) {
    sprintf((char*)D_80145AD4, D_801D0C28,
            sumLevelExpShisu00_801D5F78(index, (u8)(level + 1)));
    rowY = y;
  } else {
    sprintf((char*)D_80145AD4, D_801D0C2C);
    rowY = y + 0x30;
  }
  func_80150098((s16)(x + 0x4E), (s16)(rowY + 0x39), align, D_80145AD4);
}
