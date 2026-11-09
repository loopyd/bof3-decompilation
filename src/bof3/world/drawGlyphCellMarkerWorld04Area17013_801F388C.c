#include "bof3/bof3.h"

/* The overlay's LINE_F2 emitter (0x8017AA80 plus SetSemiTrans): it takes the
 * two corner points, then r/g/b and the semi-transparency flag. */
extern void drawSemiTransFlatLinePrimWorld04Area17013_801F3774(
    s16 x0, s16 y0, s16 x1, s16 y1, u8 r, u8 g, u8 b, u8 semi);

/* @source 0x801F388C
 * @behavior Draws the marker of one glyph cell of the overlay's 9x7 glyph grid
 *           at the signed halfword cell origin (x, y), keyed by the signed cell
 *           value the glyph table holds. Value 1 draws a cross: a 0xC8 red
 *           horizontal span from (x+2, y+5) to (x+8, y+5) and a vertical span
 *           from (x+5, y+2) to (x+5, y+8), both opaque and semi-transparent
 *           off. Value -1 draws only the horizontal span, in a 0xC8 blue.
 *           Every other value draws nothing. The 0xC8 red is held in one local
 *           so the two cross spans share it. Takes the cell origin and the
 *           signed cell value, which the caller func_801F2FA0 passes from the
 *           0x3F-byte stride glyph table at 0x801F58D0; returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawGlyphCellMarkerWorld04Area17013_801F388C(s16 x, s16 y, s8 value) {
  s32 color;

  if (value == 1) {
    color = 0xC8;
    drawSemiTransFlatLinePrimWorld04Area17013_801F3774(
        (s16)(x + 2), (s16)(y + 5), (s16)(x + 8), (s16)(y + 5), color, 0, 0, 0);
    drawSemiTransFlatLinePrimWorld04Area17013_801F3774(
        (s16)(x + 5), (s16)(y + 2), (s16)(x + 5), (s16)(y + 8), color, 0, 0, 0);
  } else if (value == -1) {
    drawSemiTransFlatLinePrimWorld04Area17013_801F3774(
        (s16)(x + 2), (s16)(y + 5), (s16)(x + 8), (s16)(y + 5), 0, 0, 0xC8, 0);
  }
}
