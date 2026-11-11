#include "bof3/world/area03004_internal.h"

/* @behavior emits one 16 by 8 textured quad of the AREA030 panel cell texture
 * into the primitive cursor: SetPolyFT4 stamps the tag, the texture page is
 * 0x229 in graphics modes 1 and 2 and 0x99 in every other mode, the CLUT is
 * 0x7A40 and the three colour bytes are the grey 0x80; the quad spans x from
 * arg0 less eight to arg0 plus eight and y from arg1 to arg1 plus eight, and its
 * texture cell is the sixteen-wide, eight-high slice at v 0x58..0x60 whose u
 * pair is (0x28,0x38) or, mirrored to (0x38,0x28), selected by the low byte of
 * the flag arg2. func_8014E5A0 then appends the 0x28-byte primitive under the
 * caller ot index arg3.
 * @source 0x801D2C34
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawMirroredPanelCellQuadWorld00Area03004_801D2C34(s16 arg0, s16 arg1,
                                                        s8 arg2, u8 arg3) {
  POLY_FT4* primitive;
  u16 tpage;

  primitive = (POLY_FT4*)g_PrimCursor;
  SetPolyFT4(primitive);

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0x229;
  } else {
    tpage = 0x99;
  }

  primitive->tpage = tpage;
  primitive->clut = 0x7A40;
  primitive->x0 = arg0 - 8;
  primitive->x1 = arg0 + 8;
  primitive->y0 = arg1;
  primitive->r0 = 0x80;
  primitive->g0 = 0x80;
  primitive->b0 = 0x80;
  primitive->y1 = arg1;
  primitive->x2 = primitive->x0;
  primitive->x3 = primitive->x1;
  primitive->y2 = primitive->y0 + 8;
  primitive->y3 = primitive->y2;

  if ((arg2 & 0xFF) == 0) {
    primitive->u0 = 0x28;
    primitive->u1 = 0x38;
  } else {
    primitive->u0 = 0x38;
    primitive->u1 = 0x28;
  }

  primitive->v0 = 0x58;
  primitive->v1 = 0x58;
  primitive->u2 = primitive->u0;
  primitive->u3 = primitive->u1;
  primitive->v2 = primitive->v0 + 8;
  primitive->v3 = primitive->v2;
  func_8014E5A0(arg3, 0x28);
}
