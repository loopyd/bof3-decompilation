#include "internal.h"

/* does: emits one textured 16x8 strip with UV orientation selected by
 * `arg2`, using the graph-type-specific tpage.
 * @source: 0x801d2c34 FUN_801d2c34
 */
void func_801d2c34(s16 arg0, s16 arg1, s8 arg2, u8 arg3) {
  POLY_FT4* primitive;

  primitive = (POLY_FT4*)BOF3_WORLD00_AREA030_PRIMITIVE_PTR;
  SetPolyFT4(primitive);

  primitive->tpage =
      (GetGraphType() == 1 || GetGraphType() == 2) ? 0x229u : 0x99u;

  primitive->clut = 0x7a40u;
  primitive->x0 = (s16)(arg0 - 8);
  primitive->x1 = (s16)(arg0 + 8);
  primitive->y0 = arg1;
  primitive->y1 = arg1;
  primitive->r0 = 0x80u;
  primitive->g0 = 0x80u;
  primitive->b0 = 0x80u;
  primitive->x2 = primitive->x0;
  primitive->x3 = primitive->x1;
  primitive->y2 = (s16)(primitive->y0 + 8);
  primitive->y3 = primitive->y2;

  if (arg2 == 0) {
    primitive->u0 = 0x28u;
    primitive->u1 = 0x38u;
  } else {
    primitive->u0 = 0x38u;
    primitive->u1 = 0x28u;
  }

  primitive->v0 = 0x58u;
  primitive->v1 = 0x58u;
  primitive->u2 = primitive->u0;
  primitive->u3 = primitive->u1;
  primitive->v2 = (u8)(primitive->v0 + 8);
  primitive->v3 = primitive->v2;

  func_8014e5a0(arg3, 0x28u);
}
