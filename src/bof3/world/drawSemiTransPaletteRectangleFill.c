#include "bof3/world/area03004_internal.h"

/* @source 0x801D934C
 * @behavior draws the semi-transparent fill of the requested rectangle in a
 *           palette colour: it reads the 15-bit entry zero of sub-palette row 1
 *           of the CLUT bank selected by the shared CLUT-bank byte D_80144952,
 *           expands it into the red, green and blue channel bytes and selects
 *           texture-page draw mode 6, then emits three semi-transparent packets
 *           from the shared primitive cursor. The leading 0x18-byte quad runs
 *           (x + 2, y), (x + w - 2, y), (x, y + 2) and (x + w, y + 2), the
 *           0x10-byte tile fills (x, y + 2) with width w and height h - 4, and
 *           the trailing quad runs (x, y + h - 2), (x + w, y + h - 2),
 *           (x + 2, y + h) and (x + w - 2, y + h), so the two quads cap the
 *           tile with two-pixel bevelled edges. Each packet is queued through
 *           func_8014E5A0(2, size).
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSemiTransPaletteRectangleFill(u16 x, u16 y, u16 w, u16 h) {
  POLY_F4* poly;
  TILE* tile;
  u32 paletteOffset;
  u16 color;
  u8 r;
  u8 g;
  u8 b;

  paletteOffset = (*(s8*)&D_80144952 << 6) | 0x20;
  color = *(u16*)(PSX_PTR(u8, 0x80033A08u) + paletteOffset);
  r = (color & 0x1f) << 3;
  g = (color >> 2) & 0xf8;
  b = (color >> 7) & 0xf8;
  submitTpageDrawMode(6, 2);

  poly = (POLY_F4*)g_PrimCursor;
  func_8017A9A4(poly);
  poly->x0 = x + 2;
  poly->x1 = x + w - 2;
  poly->x2 = x;
  poly->x3 = x + w;
  poly->y0 = y;
  poly->y1 = y;
  poly->y2 = y + 2;
  poly->y3 = y + 2;
  poly->r0 = r;
  poly->g0 = g;
  poly->b0 = b;
  SetSemiTrans(poly, 1);
  func_8014E5A0(2, 0x18);

  tile = (TILE*)g_PrimCursor;
  SetTile(tile);
  tile->x0 = x;
  tile->y0 = y + 2;
  tile->w = w;
  tile->h = h - 4;
  tile->r0 = r;
  tile->g0 = g;
  tile->b0 = b;
  SetSemiTrans(tile, 1);
  func_8014E5A0(2, 0x10);

  poly = (POLY_F4*)g_PrimCursor;
  func_8017A9A4(poly);
  poly->x0 = x;
  poly->x1 = x + w;
  poly->x2 = x + 2;
  poly->x3 = x + w - 2;
  poly->y0 = y + h - 2;
  poly->y1 = y + h - 2;
  poly->y2 = y + h;
  poly->y3 = y + h;
  poly->r0 = r;
  poly->g0 = g;
  poly->b0 = b;
  SetSemiTrans(poly, 1);
  func_8014E5A0(2, 0x18);
}
