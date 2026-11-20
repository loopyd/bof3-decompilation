#include "bof3/world/area03004_internal.h"

/* @behavior submits the AREA030 graphics-mode byte drawMode through submitTpageDrawMode,
 * then builds the 16x16 sprite primitive in place at the shared primitive
 * cursor g_PrimCursor: initSprite16Primitive sets packet length 3 and command
 * 0x7C, the colour bytes r0/g0/b0 become the gray 0x80, the CLUT halfword
 * becomes 0x7A40, the position comes from the caller halfwords (x,y) and the
 * texture cell becomes (u0,v0) = (the 0x801E1D0C byte selected by bits 3-4 of
 * the shared frame counter, 0x48); GpuAppendPrim appends the 0x10-byte
 * primitive under the caller graphics-mode byte drawMode.
 * @source 0x801D1818
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawCounterIndexedSprite16x16World00Area03004(s16 x, s16 y, u8 drawMode) {
  SPRT_16* primitive;

  submitTpageDrawMode(0, drawMode);
  primitive = (SPRT_16*)g_PrimCursor;
  initSprite16Primitive(primitive);
  primitive->r0 = 0x80;
  primitive->g0 = 0x80;
  primitive->b0 = 0x80;
  primitive->clut = 0x7a40;
  primitive->x0 = x;
  primitive->y0 = y;
  primitive->u0 = D_801E1D0C[((u32)D_80143E6C >> 3) & 3];
  primitive->v0 = 0x48;
  GpuAppendPrim(drawMode, 0x10);
}
