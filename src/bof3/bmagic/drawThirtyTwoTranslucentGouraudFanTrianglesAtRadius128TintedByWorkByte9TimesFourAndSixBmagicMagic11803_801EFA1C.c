#include "bof3/bof3.h"
#include "gpu/prim.h"

extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern s16 D_1F800024; /* @source 0x1F800024 @kind data */

s32 func_801782FC(s32 phase);
s32 func_801783C8(s32 phase);
void func_8017A97C(void *prim);
s32 GetGraphType(void);

/* @source 0x801EFA1C
 * @behavior Draws thirty-two translucent Gouraud fan triangles at radius 128,
 *           called without arguments at 0x801EECB4. Selects draw-mode tpage
 *           0x125 for graph types 1 or 2, otherwise 0x55, and links at slot 5.
 *           Reads work byte 9 into scratchpad tint halfwords 0xA and 0xC,
 *           scaled by four and six respectively. Seeds the rim at phase zero
 *           and clears the three vertex depths. Each pass carries the previous
 *           rim forward and samples the next point at step times 128, scaling
 *           signed phase samples by the radius and shifting right twelve.
 *           Projects the triplet into a POLY_G3; vertex colours are (A,A,C),
 *           (A,C,C) and (A,C,C), independently rereading the tint halfwords.
 *           Links each 0x1C-byte primitive at slot 5. Restores draw-mode tpage
 *           0x25 for graph types 1 or 2, otherwise 0x15, with another packet
 *           at slot 5. Work-field roles remain offset-anchored. Returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawThirtyTwoTranslucentGouraudFanTrianglesAtRadius128TintedByWorkByte9TimesFourAndSixBmagicMagic11803_801EFA1C(void) {
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
  GpuAppendPrim(5, 0xC);

  work = SPAD_PTR_SLOT(u8, 0x44u);
  radius = &D_1F800000;
  *radius = 128;
  SPAD_REF(u16, 0x0Au) = work[9] << 2;
  SPAD_REF(u16, 0x0Cu) = work[9] * 6;
  next = &D_1F800024;
  *next = (func_801782FC(0) * *radius) >> 12;
  SPAD_REF(s16, 0x26u) = (func_801783C8(0) * *radius) >> 12;

  step = 1;
  shape = (SVECTOR *)(next - 8);
  cells = radius;
  SPAD_REF(s16, 0x28u) = 0;
  SPAD_REF(s16, 0x20u) = 0;
  SPAD_REF(s16, 0x18u) = 0;

  for (; step < 0x21; step++) {
    prim = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);
    phase = step << 7;
    shape[0].vx = 0;
    shape[0].vy = 0;
    shape[1].vx = shape[2].vx;
    shape[1].vy = shape[2].vy;
    shape[2].vx = (func_801782FC(phase) * cells[0]) >> 12;
    shape[2].vy = (func_801783C8(phase) * cells[0]) >> 12;
    RotTransPers3(shape, shape + 1, shape + 2, (long *)&prim->x0,
                  (long *)&prim->x1, (long *)&prim->x2, &depth, &flag);
    prim->r0 = *(u16 *)(cells + 5);
    prim->g0 = *(u16 *)(cells + 5);
    prim->b0 = *(u16 *)(cells + 6);
    prim->r1 = *(u16 *)(cells + 5);
    prim->g1 = *(u16 *)(cells + 6);
    prim->b1 = *(u16 *)(cells + 6);
    prim->r2 = *(u16 *)(cells + 5);
    prim->g2 = *(u16 *)(cells + 6);
    prim->b2 = *(u16 *)(cells + 6);
    GpuAppendPrim(5, 0x1C);
  }

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0x25;
  } else {
    graph = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(5, 0xC);
}
