#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern s16 D_1F800024; /* @source 0x1F800024 @kind data */

s32 func_801782FC(s32 phase);
s32 func_801783C8(s32 phase);
void func_8017A97C(void *prim);
s32 GetGraphType(void);

/* @source 0x801EFA48
 * @behavior Draws a translucent fan of sixty-four Gouraud triangles, called
 *           at 0x801EF81C without arguments. Selects draw-mode tpage 0x125
 *           when GetGraphType returns 1 or 2 and 0x55 otherwise, then links
 *           the draw-mode packet at ordering-table slot 2. Reads the work
 *           pointer once, truncates work word 0x14 into scratchpad radius
 *           halfword 0 and stores work byte 9 times four into tint halfword 4.
 *           Seeds the rim at phase zero and clears all three vertex depths.
 *           Each pass seeds a POLY_G3, enables semitransparency, clears the
 *           origin, carries the previous rim point forward, and samples the
 *           next rim point at phase step times 64 with signed radius products
 *           shifted right twelve. RotTransPers3 projects the scratchpad
 *           triplet at 0x1F800014 into the primitive. All nine colour bytes
 *           independently read tint halfword 4, and each 0x1C-byte packet is
 *           linked at slot 2. Steps 1 through 64 close the full 4096-unit turn.
 *           Work fields retain offset-based names; their wider roles are
 *           unknown. Returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentFanOfSixtyFourGouraudTrianglesAtWorkWord14RadiusTintedByWorkByte9TimesFourBmagicMagic11003_801EFA48(void) {
  u8 *work;
  s16 *radius;
  s16 *next;
  s16 *cells;
  SVECTOR *shape;
  POLY_G3 *prim;
  long depth;
  long flag;
  s32 graph;
  s32 phase;
  s32 step;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0x125;
  } else {
    graph = 0x55;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(2, 0xC);

  work = g_battle_work;
  radius = &D_1F800000;
  *radius = *(s32 *)(work + 0x14);
  SPAD_REF(u16, 0x04u) = work[9] << 2;
  next = &D_1F800024;
  *next = (func_801782FC(0) * *radius) >> 12;
  SPAD_REF(s16, 0x26u) = (func_801783C8(0) * *radius) >> 12;

  step = 1;
  shape = (SVECTOR *)(next - 8);
  cells = radius;
  SPAD_REF(s16, 0x28u) = 0;
  SPAD_REF(s16, 0x20u) = 0;
  SPAD_REF(s16, 0x18u) = 0;

  for (; step < 0x41; step++) {
    prim = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);
    phase = step << 6;
    shape[0].vx = 0;
    shape[0].vy = 0;
    shape[1].vx = shape[2].vx;
    shape[1].vy = shape[2].vy;
    shape[2].vx = (func_801782FC(phase) * cells[0]) >> 12;
    shape[2].vy = (func_801783C8(phase) * cells[0]) >> 12;
    RotTransPers3(shape, shape + 1, shape + 2, (long *)&prim->x0,
                  (long *)&prim->x1, (long *)&prim->x2, &depth, &flag);
    prim->r0 = *(u16 *)(cells + 2);
    prim->g0 = *(u16 *)(cells + 2);
    prim->b0 = *(u16 *)(cells + 2);
    prim->r1 = *(u16 *)(cells + 2);
    prim->g1 = *(u16 *)(cells + 2);
    prim->b1 = *(u16 *)(cells + 2);
    prim->r2 = *(u16 *)(cells + 2);
    prim->g2 = *(u16 *)(cells + 2);
    prim->b2 = *(u16 *)(cells + 2);
    GpuAppendPrim(2, 0x1C);
  }
}
