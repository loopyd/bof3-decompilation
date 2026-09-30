#include "bof3/ui/shop00_internal.h"

/* @source 0x801DA00C
 * @behavior emits one 16x8 semi-transparent textured sprite at (arg0, arg1)
 *           into the shared primitive cursor: a DR_MODE carrying the
 *           graph-type texture page (0x8f for graph types 1 and 2, 0x2f
 *           otherwise), then a SPRT with width 16, height 8, texture column
 *           arg2*16, texture row 0xd8 and CLUT 0x7800. The sprite color is the
 *           dim gray 0x10 when arg3 is set, otherwise the bright gray 0x80.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DA00C(s16 arg0, s16 arg1, u8 arg2, u8 arg3) {
  SPRT* primitive;
  s32   tpage;

  if (GetGraphType() == 1) {
    tpage = 0x8f;
  } else if (GetGraphType() == 2) {
    tpage = 0x8f;
  } else {
    tpage = 0x2f;
  }

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, tpage, 0);
  appendRenderPrim(1, 0xc);

  primitive = (SPRT*)g_PrimCursor;
  primitive->w = 0x10;
  primitive->h = 8;
  primitive->clut = 0x7800;

  if (arg3 == 0) {
    primitive->r0 = primitive->g0 = primitive->b0 = 0x80;
  } else {
    primitive->r0 = primitive->g0 = primitive->b0 = 0x10;
  }

  primitive->v0 = 0xd8;
  primitive->u0 = arg2 * 16;
  primitive->x0 = arg0;
  primitive->y0 = arg1;

  SetSprt(primitive);
  SetSemiTrans(primitive, 1);
  appendRenderPrim(1, 0x14);
}
