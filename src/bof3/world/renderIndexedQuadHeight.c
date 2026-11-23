#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801D799C
 * @behavior emits one textured quad from the five AREA030 sprite-field tables
 * interleaved at 0x801E218C..0x801E2194 (record stride 0x0A: byte recordOffset
 * recordIndex * 10). The tpage field takes the 0x801E2194 halfword through the 0x3FF
 * mask and the >> 6 shift, or-ed with 0x20 for graphics modes 1/2 and with
 * 0x10 otherwise; the CLUT field is 0x7887. The four vertex fields are the
 * requested origin plus the 0x801E2190 halfword width and the requested
 * height, and the eight UV fields are the 0x801E218C / 0x801E218E halfwords
 * plus the 0x801E2190 / 0x801E2192 deltas. The quad is then submitted through
 * func_8014E5A0(1, 0x28).
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderIndexedQuadHeight(s16 x, s16 y, u8 height, u8 recordIndex)
{
  POLY_FT4* primitive;
  u16 tpage;
  u16 rightX;
  s32 recordOffset;

  primitive = (POLY_FT4*)g_PrimCursor;
  SetPolyFT4(primitive);

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = ((D_801E2194[recordIndex * 5] & 0x3ff) >> 6) | 0x20;
  } else {
    tpage = ((D_801E2194[recordIndex * 5] & 0x3ff) >> 6) | 0x10;
  }
  primitive->tpage = tpage;

  primitive->clut = 0x7887;
  recordOffset = recordIndex * 10;
  primitive->x0 = x;
  rightX = x + *(u16*)((u8*)D_801E2190 + recordOffset);
  primitive->x1 = rightX;
  primitive->x2 = primitive->x0;
  primitive->y0 = y;
  primitive->y1 = y;
  primitive->y2 = primitive->y0 + height;
  primitive->y3 = primitive->y2;
  primitive->x3 = primitive->x1;
  primitive->u0 = *(u16*)((u8*)D_801E218C + recordOffset);
  primitive->u1 = primitive->u0 + *(u16*)((u8*)D_801E2190 + recordOffset);
  primitive->u2 = primitive->u0;
  primitive->u3 = primitive->u1;
  primitive->v0 = *(u16*)((u8*)D_801E218E + recordOffset);
  primitive->v1 = primitive->v0;
  primitive->v2 = primitive->v0 + *(u8*)((u8*)D_801E2192 + recordOffset);
  primitive->v3 = primitive->v2;
  primitive->r0 = 0x80;
  primitive->g0 = 0x80;
  primitive->b0 = 0x80;
  func_8014E5A0(1, 0x28);
}
