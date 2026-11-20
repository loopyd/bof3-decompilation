#include "bof3/world/area03004_internal.h"

/* @behavior builds the AREA030 16x32 Gouraud quad in place at the shared
 * primitive cursor g_PrimCursor: SetPolyG4 stamps the tag, the upper vertex row
 * becomes (x, y) and (x + 0x10, y) with the bright 0x64/0x64/0x80
 * colour pair, the lower vertex row repeats those x positions (read back from
 * the quad) one 0x20 row down with the dim 0x00/0x00/0x50 pair, and
 * GpuAppendPrim then appends the 0x24-byte quad under the caller graphics-mode
 * byte drawMode.
 * @source 0x801D1744
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The lower-row y value goes through a named s32 temporary so both lower
 * vertex rows are stored from one computed value:
 * `y3 = (lowerY = (y2 = y0 + 0x20))`. The statement order, the stores and the
 * multiset are otherwise the lane's earlier build (which measured 44/53).
 * Without the temporary the -O2 scheduler sinks the GpuAppendPrim argument mask
 * to +0x0084 and splits the x1 store/read-back pair at +0x0080; the named
 * lifetime keeps the mask hoist at +0x0034 and the pair adjacent at +0x0040.
 * A dead `if (1) { }` guard has the same effect here but carries no behavior, so
 * it is not retained. */
void drawBlueGradientQuad16x32World00Area03004(s16 x, s16 y, u8 drawMode) {
  s32 lowerY;
  POLY_G4* primitive;

  primitive = (POLY_G4*)g_PrimCursor;
  SetPolyG4(primitive);
  primitive->x0 = x;
  primitive->x1 = x + 0x10;
  primitive->y0 = y;
  primitive->r0 = 0x64;
  primitive->g0 = 0x64;
  primitive->b0 = 0x80;
  primitive->r1 = 0x64;
  primitive->g1 = 0x64;
  primitive->b1 = 0x80;
  primitive->y1 = y;
  primitive->r2 = 0x00;
  primitive->g2 = 0x00;
  primitive->b2 = 0x50;
  primitive->r3 = 0x00;
  primitive->g3 = 0x00;
  primitive->b3 = 0x50;
  primitive->x2 = primitive->x0;
  primitive->x3 = primitive->x1;
  primitive->y3 = (lowerY = (primitive->y2 = primitive->y0 + 0x20));
  GpuAppendPrim(drawMode, 0x24);
}
