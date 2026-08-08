#include "internal.h"

/* @behavior appends a sprite primitive using indexed texture geometry. */
/* @source 0x801E0DCC */
u8* func_801E0DCC(s32 arg0, s32 arg1, s16 arg2, s16 arg3) {
  SPRT* primitive;

  primitive = (SPRT*)D_8014598C;
  SetSprt(primitive);
  primitive->r0 = 0x80;
  primitive->g0 = 0x80;
  primitive->b0 = 0x80;
  primitive->x0 = arg2;
  primitive->y0 = arg3;
  primitive->clut = (D_801E2424[(u8)arg0].clut_y << 6) |
                    ((s32)D_801E2424[(u8)arg0].clut_x >> 4 & 0x3f);
  primitive->w = D_801E2424[(u8)arg0].width;
  primitive->h = D_801E2424[(u8)arg0].height;
  primitive->u0 = D_801E2424[(u8)arg0].u;
  primitive->v0 = D_801E2424[(u8)arg0].v;
  func_8014E5A0((u8)arg1, 0x14);
  return (u8*)primitive;
}
