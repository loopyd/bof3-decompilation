#include "bof3/bof3.h"

/* Work-record table of the shared main-executable data segment: 0x140-byte rows
 * based at 0x80145E90, the rows the plchar lifts index as D_80145E90[i * 0x140]
 * and the bmagic magic069 sibling indexes as D_80145E90 + index * 0x140. This
 * overlay reaches row offset 0x74, which is where the row's NUL-terminated
 * four-character name string starts; the original scales the selector byte by
 * 0x140 for that access instead of forming the row base, so the field keeps its
 * own address-derived symbol.
 * @source 0x80145F04 @kind table
 */
extern u8 D_80145F04[];

/* Party byte of the same row, row offset 0x79 (D_80145F04 + 5). The scenario
 * lift recountLiveWorkRecordsThenCopyPartyBytesAndInvokeHelperScenarioScena1400_801FC328
 * names it the party byte at +0x79, and the bmagic magic069 sibling
 * seedWorkHalfwords2EAnd30FromSelectedBattlerRecordClassDeltaSubmitRecordStoreEffectByteAAdvanceWorkByte2BmagicMagic06903_801EF564
 * indexes the party height table with D_80145F09[index * 0x140].
 * @source 0x80145F09 @kind unknown
 */
extern u8 D_80145F09[];

/* Battle target selector byte in main RAM, outside this overlay's image. The
 * exact bmagic lifts testBattleTargetKindFlagBmagicMagic06000_801EF118 and
 * seedWorkMotionFromBattlerRecordAndAdvanceWorkByte2BmagicMagic05403_801EF238
 * read the same byte; here it selects the work record this panel reports on.
 * @source 0x80146384 @kind unknown
 */
extern u8 D_80146384;

/* Panel label strings in this overlay's own data tail, the 132-byte textbin
 * D_801F0B64 at 0x801F0B64: "WEAK" at 0x801F0B64, "EXP" at 0x801F0B6C, a
 * four-byte cell holding the single glyph code 0x60 at 0x801F0B70, "ITEM" at
 * 0x801F0B74, "POINT" at 0x801F0B7C, a four-byte cell holding the single glyph
 * code 0x5C at 0x801F0B88 and the twelve-glyph 0x5C rule at 0x801F0B8C. Every
 * one of these is a glyph-code string for the two text helpers below.
 * @source 0x801F0B64 @kind string
 */
extern u8 D_801F0B64[];

/* @source 0x801F0B6C @kind string */
extern u8 D_801F0B6C[];

/* @source 0x801F0B70 @kind string */
extern u8 D_801F0B70[];

/* @source 0x801F0B74 @kind string */
extern u8 D_801F0B74[];

/* @source 0x801F0B7C @kind string */
extern u8 D_801F0B7C[];

/* @source 0x801F0B88 @kind string */
extern u8 D_801F0B88[];

/* @source 0x801F0B8C @kind string */
extern u8 D_801F0B8C[];

/* Text helpers of this overlay. Each walks the NUL-terminated glyph-code string
 * its first argument points at and queues one textured quad per byte at the pen
 * position given by the second argument (x) and the third (y). func_801EF590
 * takes the codes from the 25-column small font bank, advances eight pixels per
 * glyph and stops after twelve glyphs or at the terminator; the first glyph
 * bank's eight-pixel cells are drawn by the sibling source this file declares
 * below, whose own metadata records every constant of that draw. */
void func_801EF590(u8 *text, s32 x, s32 y);
void drawFirstGlyphBankStringAsEightPixelCellsAtPenPositionBmagicMagic06000_801EF76C(
    u8 *text, s32 x, s32 y);

/* @source 0x801EF174
 * @behavior Draws the result panel of the current battle target: its four-
 *           character name at (0x70, 0x30) and ten static label strings of this
 *           overlay. It reads the selector byte D_80146384, takes the party byte
 *           of that selector's 0x140-byte work record at D_80145F09 (row offset
 *           0x79) into a saved register, stores zero over it, passes the same
 *           row's name string D_80145F04 (row offset 0x74) with the pen position
 *           (0x70, 0x30) to func_801EF590, re-reads the selector and stores the
 *           saved party byte back. The party byte is therefore only cleared for
 *           the name draw, and the selector is re-read for the record offset and
 *           for the restore because the intervening store may alias it.
 *           It then submits the labels, all of them this overlay's own strings:
 *           the first-glyph-bank drawer at 0x801EF76C draws "WEAK" at
 *           (0x5A, 0x44), "POINT" at (0x80, 0x44) and "ITEM" at (0x5A, 0x8A);
 *           func_801EF590 draws the twelve-glyph 0x5C
 *           rule at (0x70, 0x50), (0x70, 0x98) and (0x70, 0xAC), the one-glyph
 *           0x5C cell D_801F0B88 at (0x9C, 0x64) and (0x9C, 0x78), "EXP" at
 *           (0xB0, 0x64) and the one-glyph 0x60 cell D_801F0B70 at (0xB0, 0x78).
 *           The 0x5C and 0x60 cells and the twelve-glyph rule are the value slots
 *           of the panel, which the caller overwrites before this draw.
 *           Frame 0x20 holding $s0 and $s1, no arguments, no return value, no
 *           state of its own beyond the cleared and restored party byte.
 *           The selector is read through a byte pointer local, so one base
 *           serves all three reads, and the 0x140 product of the first two row
 *           accesses is held in its own offset local, which keeps each row
 *           access re-deriving the base instead of hoisting it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawWorkRecordNameAndBattleResultPanelLabelsBmagicMagic06000_801EF174(void) {
  u8 *sel;
  u32 offset;
  u8 party;

  sel = &D_80146384;
  offset = *sel * 0x140;
  party = D_80145F09[offset];
  D_80145F09[offset] = 0;
  func_801EF590(&D_80145F04[*sel * 0x140], 0x70, 0x30);
  D_80145F09[*sel * 0x140] = party;
  drawFirstGlyphBankStringAsEightPixelCellsAtPenPositionBmagicMagic06000_801EF76C(
      D_801F0B64, 0x5A, 0x44);
  drawFirstGlyphBankStringAsEightPixelCellsAtPenPositionBmagicMagic06000_801EF76C(
      D_801F0B7C, 0x80, 0x44);
  func_801EF590(D_801F0B8C, 0x70, 0x50);
  func_801EF590(D_801F0B88, 0x9C, 0x64);
  func_801EF590(D_801F0B6C, 0xB0, 0x64);
  func_801EF590(D_801F0B88, 0x9C, 0x78);
  func_801EF590(D_801F0B70, 0xB0, 0x78);
  drawFirstGlyphBankStringAsEightPixelCellsAtPenPositionBmagicMagic06000_801EF76C(
      D_801F0B74, 0x5A, 0x8A);
  func_801EF590(D_801F0B8C, 0x70, 0x98);
  func_801EF590(D_801F0B8C, 0x70, 0xAC);
}
