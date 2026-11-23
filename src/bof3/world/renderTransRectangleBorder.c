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
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderTransRectangleBorder(u16 x, u16 y, u16 w, u16 h, u8 mode) {
  LINE_F3* line;
  u32 paletteOffset;
  u16 color;
  u32 r;
  u32 g;
  u32 b;

  paletteOffset = (*(s8*)&D_80144952 << 6) | 0x20;
  color = *(u16*)(PSX_PTR(u8, 0x80033A08u) + paletteOffset);
  r = (color & 0x1F) << 3;
  g = (color >> 2) & 0xF8;
  b = (color >> 7) & 0xF8;

  if (mode != 0) {
    submitTpageDrawMode(7, 2);
  } else {
    submitTpageDrawMode(6, 2);
  }
  line = (LINE_F3*)g_PrimCursor;
  SetLineF3(line);
  line->x1 = x + w;
  line->x2 = x + w;
  line->x0 = x;
  line->y0 = y + h;
  line->y1 = y + h;
  line->y2 = y;
  line->r0 = r;
  line->g0 = g;
  line->b0 = b;
  SetSemiTrans(line, 1);
  func_8014E5A0(2, 0x18);

  if (mode != 0) {
    submitTpageDrawMode(6, 2);
  } else {
    submitTpageDrawMode(7, 2);
  }
  line = (LINE_F3*)g_PrimCursor;
  SetLineF3(line);
  line->x0 = x;
  line->x1 = x;
  line->x2 = x + w;
  line->y0 = y + h;
  line->y1 = y;
  line->y2 = y;
  line->r0 = r;
  line->g0 = g;
  line->b0 = b;
  SetSemiTrans(line, 1);
  func_8014E5A0(2, 0x18);
}
