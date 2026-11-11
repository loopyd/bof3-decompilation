#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801D8248
 * @behavior emits one grey textured quad into the primitive cursor at the requested
 * origin: a column eight pixels wide and of the requested height, whose
 * texels are the 8 by 4 slice at (0x58, 0x9A) of the panel texture page that
 * the tpage field 0x2D selects in graphics modes 1 and 2 and 0x1D in every
 * other mode, with CLUT 0x7887 and the three colour bytes 0x80. The source V
 * fields are the base 0x9A and the base plus four, so the four-texel band is
 * stretched over the requested height; the quad is submitted through
 * func_8014E5A0(1, 0x28).
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawStretchedPanelColumnWorld00Area03004_801D8248(s16 x, s16 y, u8 height) {
  POLY_FT4* primitive;
  u16 tpage;

  primitive = (POLY_FT4*)g_PrimCursor;
  SetPolyFT4(primitive);

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0x2D;
  } else {
    tpage = 0x1D;
  }

  primitive->x0 = x;
  primitive->tpage = tpage;
  primitive->clut = 0x7887;
  primitive->x1 = x + 8;
  primitive->u0 = 0x58;
  primitive->u1 = 0x60;
  primitive->v0 = 0x9A;
  primitive->v1 = 0x9A;
  primitive->y0 = y;
  primitive->r0 = 0x80;
  primitive->g0 = 0x80;
  primitive->b0 = 0x80;
  primitive->y1 = y;
  primitive->x2 = primitive->x0;
  primitive->x3 = primitive->x1;
  primitive->y2 = primitive->y0 + height;
  primitive->y3 = primitive->y2;
  primitive->u2 = primitive->u0;
  primitive->u3 = primitive->u1;
  primitive->v2 = primitive->v0 + 4;
  primitive->v3 = primitive->v2;
  func_8014E5A0(1, 0x28);
}
