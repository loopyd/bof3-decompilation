#include "bof3/context.h"
#include "gpu/prim.h"

/* @source 0x8014E5A0 */
extern void appendRenderPrim(u32 ot_index, u32 primitive_size);

/* @source 0x801D1714
 * @behavior draws a zero-terminated byte string as skewed 8x8 textured quads,
 * advancing eight pixels per byte and skipping spaces. The low six palette
 * bits select a CLUT on row 0x1E0; every quad uses texture page 0xF.
 * @status partial
 * @match 98.04
 * @residual entry argument-copy order differs; all later instructions match.
 */
void drawGlyphStringQuadsCommu03_06(s32 x, s32 y, u32 palette, const u8* text) {
  POLY_FT4* primitive;
  u16 clut;

  clut = (palette & 0x3F) | 0x7800;
  while (*(u8*)text != 0) {
    if (*(u8*)text != ' ') {
      primitive = (POLY_FT4*)g_PrimCursor;
      SetPolyFT4(primitive);
      primitive->x0 = x + 4;
      primitive->x1 = x + 12;
      primitive->y0 = y;
      primitive->y1 = y;
      primitive->x2 = primitive->x0 - 4;
      primitive->x3 = primitive->x1 - 4;
      primitive->y2 = primitive->y0 + 8;
      primitive->y3 = primitive->y2;
      {
        s32 byte = *(u8*)text;
        s32 glyph = byte - 0x20;
        s32 adjusted = glyph;
        if (adjusted < 0) {
          adjusted = byte - 1;
        }
        primitive->u0 = (glyph - ((adjusted >> 5) * 32)) * 8;
      }
      primitive->u1 = primitive->u0 + 7;
      primitive->u2 = primitive->u0;
      primitive->u3 = primitive->u1;
      {
        s32 byte = *(u8*)text;
        s32 adjusted = byte - 0x20;
        if (adjusted < 0) {
          adjusted = byte - 1;
        }
        primitive->v0 = (adjusted >> 5) * 8;
      }
      primitive->v1 = primitive->v0;
      primitive->v2 = primitive->v0 + 8;
      primitive->v3 = primitive->v2;
      primitive->r0 = 0x80;
      primitive->g0 = 0x80;
      primitive->b0 = 0x80;
      primitive->clut = clut;
      if (GetGraphType() != 1) {
        GetGraphType();
      }
      primitive->tpage = 0xF;
      appendRenderPrim(1, 0x28);
    }
    text++;
    x += 8;
  }
}
