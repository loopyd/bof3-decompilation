#include "bof3/bof3.h"

/* Battle target selector byte in main RAM, outside this overlay's image. The
 * exact siblings testBattleTargetKindFlagBmagicMagic06000_801EF118 and
 * drawWorkRecordNameAndBattleResultPanelLabelsBmagicMagic06000_801EF174 read
 * the same cell, and the exact battle lift drawBattlerStatusPanel subtracts the
 * same 3 from its panel index before scaling it by the record stride.
 * @source 0x80146384 @kind unknown
 */
extern u8 D_80146384;

/* Row base of the 0x118-byte local panel record table of the shared main
 * executable, the row of the battle target selected by (selector - 3). Its
 * leading field is the up-to-eight-cell glyph name this panel draws, which the
 * exact sibling drawUpToEightAsciiGlyphBankCellsAtPenPositionBmagicMagic06000_801EF924
 * records, and the exact battle lift drawBattlerStatusPanel indexes the same row
 * base as D_801EB6A4[index * 0x118].
 * @source 0x801EB6A4 @kind unknown
 */
extern u8 D_801EB6A4[];

/* Byte at row offset 0xB of the same record: nonzero when the row carries the
 * two numbers, zero when the panel falls back to the 0x5C placeholder cells.
 * The battle lift drawBattlerStatusPanel tests the same byte of its own record
 * table for equality with 1; here the test is a plain zero test, so the two
 * spans are different walks of one record shape.
 * @source 0x801EB6AF @kind unknown
 */
extern u8 D_801EB6AF[];

/* @source 0x801EB6B4 @kind unknown */
extern u8 D_801EB6B4[];

/* @source 0x801EB6B6 @kind unknown */
extern u8 D_801EB6B6[];

/* @source 0x801EB6C8 @kind unknown */
extern u8 D_801EB6C8[];

/* @source 0x801EB6CC @kind unknown */
extern u8 D_801EB6CC[];

/* Panel label strings of this overlay's own data tail: "EXP" at 0x801F0B6C, the
 * single glyph code 0x60 at 0x801F0B70, "ITEM" at 0x801F0B74 and the single
 * glyph code 0x5C at 0x801F0B88, the placeholder the two number rows fall back
 * to. The exact sibling drawWorkRecordNameAndBattleResultPanelLabelsBmagicMagic06000_801EF174
 * draws the same four cells at the same pen rows.
 * @source 0x801F0B6C @kind string
 */
extern u8 D_801F0B6C[];

/* @source 0x801F0B70 @kind string */
extern u8 D_801F0B70[];

/* @source 0x801F0B74 @kind string */
extern u8 D_801F0B74[];

/* @source 0x801F0B88 @kind string */
extern u8 D_801F0B88[];

/* Text helpers of this overlay. Each walks the NUL-terminated glyph-code string
 * its first argument points at and queues one textured quad per byte at the pen
 * position given by the second argument (x) and the third (y), taking the codes
 * from an 8-by-12 cell bank.
 * drawUpToEightAsciiGlyphBankCellsAtPenPositionBmagicMagic06000_801EF924 stops
 * after eight cells, func_801EF590 after twelve, and
 * drawFirstGlyphBankStringAsEightPixelCellsAtPenPositionBmagicMagic06000_801EF76C
 * draws the first glyph bank's eight-pixel cells.
 * drawKindSelectedValueTextOrRulePlaceholderBmagicMagic06000_801EF470 takes the
 * two bytes of one value halfword as (low byte, high byte) and the row y.
 * func_801EFB00 is the decimal drawer of the same bank: it walks the five digit
 * places 10000 down to 1 of its first argument, which it keeps word-wide (the
 * andi 0xFFFF inside it is its own), suppressing leading zero digits and
 * advancing the pen by 0xC per place. */
void drawUpToEightAsciiGlyphBankCellsAtPenPositionBmagicMagic06000_801EF924(
    u8 *text, s32 x, s32 y);
void func_801EF590(u8 *text, s32 x, s32 y);
void drawFirstGlyphBankStringAsEightPixelCellsAtPenPositionBmagicMagic06000_801EF76C(
    u8 *text, s32 x, s32 y);
void drawKindSelectedValueTextOrRulePlaceholderBmagicMagic06000_801EF470(
    u8 code, u8 kind, s32 y);
void func_801EFB00(u16 value, s32 x, s32 y);

/* @source 0x801EF2DC
 * @behavior Result panel of the non-party battle target, the arm the exact
 *           sibling drawPartyOrEnemyTargetResultPanelThenSetWorkByteBTo83AndAdvanceWorkByte1WhenPadStateSetBmagicMagic06000_801EEE84
 *           takes when the selector byte D_80146384 reaches 3. It scales
 *           (selector - 3) by the 0x118-byte stride of the local panel record
 *           table at 0x801EB6A4 into one offset local and draws that row: its
 *           up-to-eight-cell name at the pen (0x70, 0x30) first, on both arms.
 *           With the row byte at +0xB set it then draws the row's two numbers
 *           through func_801EFB00 at (0x70, 0x64) and (0x70, 0x78) with the
 *           "EXP" label between them at (0xB0, 0x64), the numbers coming from
 *           the halfwords at row offsets 0x12 and 0x10; with that byte clear it
 *           draws the one-glyph 0x5C placeholder cell D_801F0B88 in their place
 *           at (0x9C, 0x64) and (0x9C, 0x78) and keeps only the "EXP" label.
 *           Both arms then converge, which is why the offset is re-derived from
 *           the saved selector local after the calls instead of being kept
 *           across them: the clear arm reuses that saved register for the
 *           placeholder cell, so the entry-block product does not survive the
 *           join. From there the panel is the same as the party sibling: the
 *           one-glyph 0x60 cell D_801F0B70 at (0xB0, 0x78), "ITEM" at
 *           (0x5A, 0x8A), and the row's two value halfwords at offsets 0x1C and
 *           0x20 submitted through
 *           drawKindSelectedValueTextOrRulePlaceholderBmagicMagic06000_801EF470
 *           as (low byte, high byte) at the value rows 0x98 and 0xAC.
 *           Frame 0x20 holding $ra at 0x18, $s1 at 0x14 and $s0 at 0x10; no
 *           arguments, no return value and no state written outside the
 *           primitive cursor. Both call sites pass it nothing: the exact
 *           sibling above and the sibling at 0x801EEE00 reach it
 *           from the same (selector < 3) partition with a nop in the jal delay
 *           slot, so the parameter roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawEnemyTargetResultPanelNameNumbersAndValueRowsBmagicMagic06000_801EF2DC(
    void) {
  u8 index;
  u16 value;

  index = D_80146384 - 3;
  if (D_801EB6AF[index * 0x118] != 0) {
    drawUpToEightAsciiGlyphBankCellsAtPenPositionBmagicMagic06000_801EF924(
        &D_801EB6A4[index * 0x118], 0x70, 0x30);
    func_801EFB00(*(u16 *)(D_801EB6B6 + index * 0x118), 0x70, 0x64);
    func_801EF590(D_801F0B6C, 0xB0, 0x64);
    func_801EFB00(*(u16 *)(D_801EB6B4 + index * 0x118), 0x70, 0x78);
  } else {
    drawUpToEightAsciiGlyphBankCellsAtPenPositionBmagicMagic06000_801EF924(
        &D_801EB6A4[index * 0x118], 0x70, 0x30);
    func_801EF590(D_801F0B88, 0x9C, 0x64);
    func_801EF590(D_801F0B6C, 0xB0, 0x64);
    func_801EF590(D_801F0B88, 0x9C, 0x78);
  }
  func_801EF590(D_801F0B70, 0xB0, 0x78);
  drawFirstGlyphBankStringAsEightPixelCellsAtPenPositionBmagicMagic06000_801EF76C(
      D_801F0B74, 0x5A, 0x8A);
  value = *(u16 *)(D_801EB6C8 + index * 0x118);
  drawKindSelectedValueTextOrRulePlaceholderBmagicMagic06000_801EF470(
      value & 0xFF, value >> 8, 0x98);
  value = *(u16 *)(D_801EB6CC + index * 0x118);
  drawKindSelectedValueTextOrRulePlaceholderBmagicMagic06000_801EF470(
      value & 0xFF, value >> 8, 0xAC);
}
