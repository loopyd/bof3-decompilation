#include "bof3/bof3.h"

/* The overlay primitive emitters, declared as the sibling sources of this
 * target declare them: 0x801F3594 fills a POLY_F4, 0x801F3774 a LINE_F2 and
 * 0x801F3838 arms the draw mode of the overlay texture page. */
extern void func_801F3838(s32 mode);
extern void func_801F3594(s16 x0, s16 y0, s16 x1, s16 y1, s16 x2, s16 y2,
                          s16 x3, s16 y3, u8 r, u8 g, u8 b, u8 semi);
extern void func_801F3774(s16 x0, s16 y0, s16 x1, s16 y1, u8 r, u8 g, u8 b,
                          u8 semi);
void drawGlyphCellMarkerWorld04Area17013_801F388C(s16 x, s16 y, s8 value);

/* @source 0x801F2FA0
 * @behavior Draws one glyph of the overlay banner as a 9-by-7 grid of
 *           10-pixel cells at the signed halfword origin (x, y). Arms draw
 *           mode 2 and covers the 0x5A by 0x46 glyph box with one
 *           semi-transparent (0x40, 0x40, 0x40) POLY_F4, then arms draw mode 1
 *           and draws the cell borders as ten gray (0x96) vertical lines at
 *           x+10k running from y to y+0x45 and eight at y+10k running from x
 *           to x+0x59. It then marks every cell of the grid: display column j
 *           at x+10j takes its seven row values from the 63-byte signed glyph
 *           table that is passed in, read at row*9+phase, with the phase
 *           advanced by one per display column and wrapped to zero once it
 *           reaches nine, so column j carries glyph column (phase0+j) mod 9.
 *           Each value goes to the marker routine at 0x801F388C, whose 1 draws
 *           a cross and whose -1 draws a blue bar. Takes the signed halfword
 *           origin, the signed glyph table and the starting column phase;
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Matching shape: the origin arrives as signed halfwords and every use casts
 * back to s16, so the entry block keeps one full-width copy of each parameter
 * for the box call and a second one for the three cell loops. The marker loop
 * carries its own column counter: a counter shared with the vertical-line loop
 * puts both in one register, where the original gives each loop its own, while
 * the row counter serves both the horizontal-line loop and the marker rows.
 */
void drawGlyphGridAndCellMarkersWorld04Area17013_801F2FA0(s16 x, s16 y,
                                                          s8* glyphs,
                                                          u8 phase) {
  u8 col;
  u8 row;
  u8 markCol;

  func_801F3838(2);
  func_801F3594((s16)x, (s16)y, (s16)(x + 0x5A), (s16)y, (s16)x,
                (s16)(y + 0x46), (s16)(x + 0x5A), (s16)(y + 0x46), 0x40, 0x40,
                0x40, 1);
  func_801F3838(1);

  for (col = 0; col < 0xA; col++) {
    func_801F3774((s16)(x + col * 0xA), (s16)y, (s16)(x + col * 0xA),
                  (s16)(y + 0x45), 0x96, 0x96, 0x96, 0);
  }

  for (row = 0; row < 0x8; row++) {
    func_801F3774((s16)x, (s16)(y + row * 0xA), (s16)(x + 0x59),
                  (s16)(y + row * 0xA), 0x96, 0x96, 0x96, 0);
  }

  for (markCol = 0; markCol < 0x9; markCol++) {
    for (row = 0; row < 0x7; row++) {
      drawGlyphCellMarkerWorld04Area17013_801F388C(
          (s16)(x + markCol * 0xA), (s16)(y + row * 0xA),
          glyphs[row * 0x9 + phase]);
    }
    phase++;
    if (phase >= 0x9) {
      phase = 0;
    }
  }
}
