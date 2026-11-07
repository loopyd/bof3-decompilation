#include "bof3/bof3.h"

/* The overlay's primitive emitters. 0x801F3594 fills a POLY_F4 (SetPolyF4 at
 * 0x8017A9A4 plus SetSemiTrans), 0x801F3698 a POLY_F3 (SetPolyF3 at 0x8017A954)
 * and 0x801F3774 a LINE_F2 (0x8017AA80); each takes the corner coordinates, then
 * r/g/b and the semi-transparency flag, and hands the primitive to 0x8014E5A0
 * with its own size. 0x801F2FA0 draws one 63-byte glyph as its 9x7 grid of
 * cells starting at the given row phase, and 0x801F3838 arms the draw mode of
 * the texture page at (0x380, 0x100). */
extern void func_801F3838(s32 mode);
extern void func_801F3594(s16 x0, s16 y0, s16 x1, s16 y1, s16 x2, s16 y2,
                          s16 x3, s16 y3, u8 r, u8 g, u8 b, u8 semi);
extern void func_801F3698(s16 x0, s16 y0, s16 x1, s16 y1, s16 x2, s16 y2, u8 r,
                          u8 g, u8 b, u8 semi);
extern void func_801F3774(s16 x0, s16 y0, s16 x1, s16 y1, u8 r, u8 g, u8 b,
                          u8 semi);
extern void func_801F2FA0(s16 x, s16 y, u8* glyphs, u8 phase);

extern u8 D_801F58D0[]; /* @source 0x801F58D0 @kind data */
extern u8 D_801F5990;   /* @source 0x801F5990 @kind data */
extern u8 D_801F5991;   /* @source 0x801F5991 @kind data */
extern u8 D_801F5992;   /* @source 0x801F5992 @kind data */
extern u32 D_80143E6C;  /* @source 0x80143E6C @kind data */

/* The scratchpad work-record pointer cell, named so that each of the three mark
 * groups re-materialises it instead of sharing one hoisted base. */
extern u8* D_1F800044;

/* @source 0x801F31DC
 * @behavior Draws the overlay's three-glyph banner and marks the glyph the work
 *           record selected. Arms draw mode 0, then covers the box anchored at
 *           (x, y) with three semi-transparent (0, 0x80, 0) POLY_F4 pieces, the
 *           whole 0x136 wide and 0x5A tall: a 5-pixel left cap through (x, y+5),
 *           (x+5, y), (x, y+0x55) and (x+5, y+0x5A), the body from x+5 to
 *           x+0x131 between y and y+0x5A, and the mirrored right cap. It then
 *           draws the three 9x7 glyph grids at x+9, x+0x6D and x+0xD1 (100
 *           apart, all at y+0xA) from the 0x3F-byte stride table at 0x801F58D0,
 *           each starting at the row phase held in the overlay bytes 0x801F5990,
 *           0x801F5991 and 0x801F5992. When bit 4 of the shared frame word
 *           0x80143E6C is set it marks the selected glyph instead of returning:
 *           byte 6 of the scratchpad work record at 0x1F800044 is the glyph
 *           index, scaled by 100 to reach that glyph's column, and the marks are
 *           placed one pixel left of the banner origin. Two unfilled
 *           (0xC8, 0xC8, 0xC8) triangles flank the column as
 *           (c+7, y+0x29)/(c+3, y+0x2D)/(c+7, y+0x31) and
 *           (c+0x68, y+0x29)/(c+0x6C, y+0x2D)/(c+0x68, y+0x31) with c the
 *           column origin plus one, then four lines draw the 0x5A by 0x46
 *           selection frame from c+0xA at y+0xA to c+0x64 at y+0x50. The record
 *           is re-read for each of the three marks, so its cell is loaded once
 *           per group. Takes the banner's signed halfword origin in x and y,
 *           which the caller func_801F2D80 passes from the work record's
 *           halfwords 0x2E and 0x30; returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Matching shape: the one-pixel shift is its own local computed before the
 * frame-word test rather than a decrement of x inside the guarded block. That
 * keeps the x parameter a single-block quantity, which is what leaves s4 for
 * the shifted origin and moves the (y+0x0A) glyph row into s5. Each mark
 * group's offset and column local is also scoped to that group's own compound
 * statement: declared once for the whole block, the column chain lands in the
 * register pair the original gives its cell pointer and row phase instead. Both
 * forms keep the recomputed 100-scaled index per group, so the work record is
 * re-read three times as the original does.
 */
void drawThreeGlyphPanelAndSelectionCursorWorld04Area17013_801F31DC(s32 x,
                                                                    s32 y) {
  s32 origin;

  func_801F3838(0);

  func_801F3594((s16)x, (s16)(y + 0x05), (s16)(x + 0x05), (s16)y, (s16)x,
                (s16)(y + 0x55), (s16)(x + 0x05), (s16)(y + 0x5A), 0x00, 0x80,
                0x00, 0x01);
  func_801F3594((s16)(x + 0x05), (s16)y, (s16)(x + 0x131), (s16)y,
                (s16)(x + 0x05), (s16)(y + 0x5A), (s16)(x + 0x131),
                (s16)(y + 0x5A), 0x00, 0x80, 0x00, 0x01);
  func_801F3594((s16)(x + 0x131), (s16)y, (s16)(x + 0x136), (s16)(y + 0x05),
                (s16)(x + 0x131), (s16)(y + 0x5A), (s16)(x + 0x136),
                (s16)(y + 0x55), 0x00, 0x80, 0x00, 0x01);

  func_801F2FA0((s16)(x + 0x09), (s16)(y + 0x0A), D_801F58D0, D_801F5990);
  func_801F2FA0((s16)(x + 0x6D), (s16)(y + 0x0A), D_801F58D0 + 0x3F, D_801F5991);
  func_801F2FA0((s16)(x + 0xD1), (s16)(y + 0x0A), D_801F58D0 + 0x7E, D_801F5992);

  origin = x - 1;

  if (((u32)D_80143E6C >> 4) & 1u) {
    {
      s32 cell;

      cell = origin + D_1F800044[6] * 100;
      func_801F3698((s16)(cell + 7), (s16)(y + 0x29), (s16)(cell + 3),
                    (s16)(y + 0x2D), (s16)(cell + 7), (s16)(y + 0x31), 0xC8,
                    0xC8, 0xC8, 0x00);
    }
    {
      s32 cell;
      s32 offset;

      offset = D_1F800044[6] * 100 + 0x64;
      cell = origin + offset;
      func_801F3698((s16)(cell + 4), (s16)(y + 0x29), (s16)(cell + 8),
                    (s16)(y + 0x2D), (s16)(cell + 4), (s16)(y + 0x31), 0xC8,
                    0xC8, 0xC8, 0x00);
    }
    {
      s32 cell;
      s32 offset;

      offset = D_1F800044[6] * 100 + 0x0A;
      cell = origin + offset;
      func_801F3774((s16)cell, (s16)(y + 0x0A), (s16)(cell + 0x5A),
                    (s16)(y + 0x0A), 0xC8, 0xC8, 0xC8, 0x00);
      func_801F3774((s16)cell, (s16)(y + 0x0A), (s16)cell, (s16)(y + 0x50),
                    0xC8, 0xC8, 0xC8, 0x00);
      func_801F3774((s16)cell, (s16)(y + 0x50), (s16)(cell + 0x5A),
                    (s16)(y + 0x50), 0xC8, 0xC8, 0xC8, 0x00);
      func_801F3774((s16)(cell + 0x5A), (s16)(y + 0x0A), (s16)(cell + 0x5A),
                    (s16)(y + 0x50), 0xC8, 0xC8, 0xC8, 0x00);
    }
  }
}
