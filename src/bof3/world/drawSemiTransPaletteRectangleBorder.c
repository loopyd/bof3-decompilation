#include "bof3/world/area03004_internal.h"

/* Palette base of the main executable, outside this EMI payload: the shipped
 * code indexes it with the byte offset of a 16-colour sub-palette row and reads
 * the first 15-bit colour of that row.
 * @source 0x80033A08 @kind table */
extern u16 D_80033A08[];

/* @source 0x801D95C4
 * @behavior draws the semi-transparent border of the requested rectangle in a
 *           palette colour: it reads the 15-bit colour entry zero of the
 *           sub-palette row 1 of the CLUT bank selected by the shared
 *           CLUT-bank byte D_80144952, expands it into the red, green and blue
 *           channel bytes, then emits two semi-transparent flat three-point
 *           lines from the shared primitive cursor, the first through
 *           (x, y + h), (x + w, y + h) and (x + w, y) and the second through
 *           (x, y + h), (x, y) and (x + w, y), so the pair covers the four
 *           sides of the rectangle. The trailing byte selects the texture-page
 *           draw mode of the first line, 7 when it is set and 6 when it is
 *           clear, and the opposite mode for the second line; each 0x18-byte
 *           packet is queued through func_8014E5A0(2, 0x18).
 * @status partial
 * @match 39.80
 * @residual the palette read, both tpage-mode selections, the two calls and
 *           every packet store agree with the original, but the shipped body
 *           recomputes each coordinate sum inside its own packet while this
 *           candidate keeps one x + w and one y + h for both packets: the
 *           longer live ranges force a saved copy of x and of y and one
 *           spilled channel byte (98 instructions and 392 bytes against 95
 *           and 380).
 */
void drawSemiTransPaletteRectangleBorder(u16 x, u16 y, u16 w, u16 h, u8 mode) {
  LINE_F3* line;
  u32 paletteOffset;
  u16 color;
  u32 r;
  u32 g;
  u32 b;
  u16 right;
  u16 bottom;

  paletteOffset = (*(s8*)&D_80144952 << 6) | 0x20;
  color = *(u16*)((u8*)D_80033A08 + paletteOffset);
  r = (color & 0x1F) << 3;
  g = (color >> 2) & 0xF8;
  b = (color >> 7) & 0xF8;

  submitTpageDrawMode(mode != 0 ? 7 : 6, 2);
  line = (LINE_F3*)g_PrimCursor;
  SetLineF3(line);
  right = x + w;
  bottom = y + h;
  line->x1 = right;
  line->x2 = right;
  line->x0 = x;
  line->y0 = bottom;
  line->y1 = bottom;
  line->y2 = y;
  line->r0 = r;
  line->g0 = g;
  line->b0 = b;
  SetSemiTrans(line, 1);
  func_8014E5A0(2, 0x18);

  submitTpageDrawMode(mode != 0 ? 6 : 7, 2);
  line = (LINE_F3*)g_PrimCursor;
  SetLineF3(line);
  right = x + w;
  bottom = y + h;
  line->x2 = right;
  line->x0 = x;
  line->x1 = x;
  line->y0 = bottom;
  line->y1 = y;
  line->y2 = y;
  line->r0 = r;
  line->g0 = g;
  line->b0 = b;
  SetSemiTrans(line, 1);
  func_8014E5A0(2, 0x18);
}
