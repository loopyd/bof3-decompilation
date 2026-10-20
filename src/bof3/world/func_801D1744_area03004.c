#include "bof3/world/area03004_internal.h"

/* @behavior builds the AREA030 16x32 Gouraud quad in place at the shared
 * primitive cursor g_PrimCursor: SetPolyG4 stamps the tag, the upper vertex row
 * becomes (arg0, arg1) and (arg0 + 0x10, arg1) with the bright 0x64/0x64/0x80
 * colour pair, the lower vertex row repeats those x positions (read back from
 * the quad) one 0x20 row down with the dim 0x00/0x00/0x50 pair, and
 * GpuAppendPrim then appends the 0x24-byte quad under the caller graphics-mode
 * byte arg2.
 * @source 0x801D1744
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The lower-row y value goes through a named s32 temporary so both lower
 * vertex rows are stored from one computed value:
 * `y3 = (y_lower = (y2 = y0 + 0x20))`. The statement order, the stores and the
 * multiset are otherwise the lane's earlier build (which measured 44/53).
 * Without the temporary the -O2 scheduler sinks the GpuAppendPrim argument mask
 * to +0x0084 and splits the x1 store/read-back pair at +0x0080; the named
 * lifetime keeps the mask hoist at +0x0034 and the pair adjacent at +0x0040.
 * A dead `if (1) { }` guard has the same effect here but carries no behavior, so
 * it is not retained. */
void func_801D1744(s16 arg0, s16 arg1, u8 arg2) {
  s32 y_lower;
  POLY_G4* primitive;

  primitive = (POLY_G4*)g_PrimCursor;
  SetPolyG4(primitive);
  primitive->x0 = arg0;
  primitive->x1 = arg0 + 0x10;
  primitive->y0 = arg1;
  primitive->r0 = 0x64;
  primitive->g0 = 0x64;
  primitive->b0 = 0x80;
  primitive->r1 = 0x64;
  primitive->g1 = 0x64;
  primitive->b1 = 0x80;
  primitive->y1 = arg1;
  primitive->r2 = 0x00;
  primitive->g2 = 0x00;
  primitive->b2 = 0x50;
  primitive->r3 = 0x00;
  primitive->g3 = 0x00;
  primitive->b3 = 0x50;
  primitive->x2 = primitive->x0;
  primitive->x3 = primitive->x1;
  primitive->y3 = (y_lower = (primitive->y2 = primitive->y0 + 0x20));
  GpuAppendPrim(arg2, 0x24);
}
