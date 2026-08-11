#include "bof3/world/area03004_internal.h"

/* @source 0x801D799C
 * @behavior emits one textured 40-byte quad from a ten-byte sprite record.
 * @status partial
 * @match 65.22
 * @residual size, branch shape, and sprite-record load scheduling differ
 */
void func_801D799C(s16 x, s16 y, u8 height, u8 index)
{
  POLY_FT4* primitive;
  s32 offset;
  u16 x1;
  u8 y1;

  primitive = (POLY_FT4*)g_PrimCursor;
  SetPolyFT4(primitive);

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    primitive->tpage = ((D_801E2194[index * 5] & 0x3ff) >> 6) | 0x20;
  } else {
    primitive->tpage = ((D_801E2194[index * 5] & 0x3ff) >> 6) | 0x10;
  }

  primitive->clut = 0x7887;
  offset = index * 5;
  primitive->x0 = x;
  x1 = x + D_801E2190[offset];
  primitive->y0 = y;
  primitive->y1 = y;
  primitive->x2 = primitive->x0;
  primitive->x1 = x1;
  primitive->x3 = primitive->x1;
  y1 = y + height;
  primitive->y2 = y1;
  primitive->y3 = y1;
  primitive->u0 = D_801E218C[offset];
  primitive->u1 = D_801E218C[offset] + D_801E2190[offset];
  primitive->u2 = primitive->u0;
  primitive->u3 = primitive->u1;
  primitive->v0 = D_801E218E[offset];
  primitive->v1 = D_801E218E[offset];
  primitive->v2 = D_801E218E[offset] + D_801E2192[offset];
  primitive->v3 = primitive->v2;
  primitive->r0 = 0x80;
  primitive->g0 = 0x80;
  primitive->b0 = 0x80;
  func_8014E5A0(1, 0x28);
}
