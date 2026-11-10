#include "bof3/bof3.h"
#include "gpu/prim.h"

extern s16 D_1F800004; /* @source 0x1F800004 @kind data */
extern s16 D_1F800002; /* @source 0x1F800002 @kind data */
extern s16 D_1F80001C; /* @source 0x1F80001C @kind data */

s32 func_801782FC(s32 phase);
s32 func_801783C8(s32 phase);
s32 GetGraphType(void);

/* @source 0x801EF6BC
 * @behavior Draws thirty-two translucent Gouraud quads between radii 128 and
 *           192, called without arguments at 0x801EECAC after the work-record
 *           transform is installed. Selects draw-mode tpage 0x125 for graph
 *           types 1 or 2, otherwise 0x55, and links at slot 5. Seeds the inner
 *           and outer radii at scratchpad halfwords 2 and 4, and stores work
 *           byte 9 times six as the tint halfword at 0xA. Primes both edges at
 *           phase zero and clears all four vertex depths. Each pass carries
 *           both edges forward and samples the next points at step times 128,
 *           scaling signed phase samples by the radii and shifting right
 *           twelve. Projects through RotTransPers4, colours the outer edge
 *           RGB 1 and the inner edge with six independent tint reads, then
 *           links the 0x24-byte POLY_G4 at slot 5. Restores draw-mode tpage
 *           0x25 for graph types 1 or 2, otherwise 0x15, with another packet
 *           at slot 5. Takes no arguments and returns nothing; the wider role
 *           of the work field remains unknown.
 * @status partial
 * @match 99.07
 * @residual Work-pointer load follows the first radius store instead of preceding it.
 */
void drawThirtyTwoTranslucentGouraudQuadsBetweenRadii128And192TintingInnerEdgeByWorkByte9TimesSixBmagicMagic11803_801EF6BC(void) {
  u8 *work;
  s16 *radius;
  s16 *next;
  s16 *cells;
  SVECTOR *shape;
  POLY_G4 *prim;
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
  radius = &D_1F800002;
  *radius = 128;
  D_1F800004 = 192;
  SPAD_REF(u16, 0x0Au) = work[9] * 6;
  next = &D_1F80001C;
  *next = (func_801782FC(0) * D_1F800004) >> 12;
  SPAD_REF(s16, 0x1Eu) = (func_801783C8(0) * D_1F800004) >> 12;
  SPAD_REF(s16, 0x2Cu) = (func_801782FC(0) * *radius) >> 12;
  SPAD_REF(s16, 0x2Eu) = (func_801783C8(0) * *radius) >> 12;

  step = 1;
  shape = (SVECTOR *)(next - 4);
  cells = radius + 1;
  SPAD_REF(s16, 0x30u) = 0;
  SPAD_REF(s16, 0x28u) = 0;
  SPAD_REF(s16, 0x20u) = 0;
  SPAD_REF(s16, 0x18u) = 0;

  for (; step < 0x21; step++) {
    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    phase = step << 7;
    shape[0].vx = shape[1].vx;
    shape[0].vy = shape[1].vy;
    shape[2].vx = shape[3].vx;
    shape[2].vy = shape[3].vy;
    shape[1].vx = (func_801782FC(phase) * cells[0]) >> 12;
    shape[1].vy = (func_801783C8(phase) * cells[0]) >> 12;
    shape[3].vx = (func_801782FC(phase) * cells[-1]) >> 12;
    shape[3].vy = (func_801783C8(phase) * cells[-1]) >> 12;
    RotTransPers4(shape, shape + 1, shape + 2, shape + 3, (long *)&prim->x0,
                  (long *)&prim->x1, (long *)&prim->x2, (long *)&prim->x3,
                  &depth, &flag);
    prim->r0 = 1;
    prim->g0 = 1;
    prim->b0 = 1;
    prim->r1 = 1;
    prim->g1 = 1;
    prim->b1 = 1;
    prim->r2 = *(u16 *)(cells + 3);
    prim->g2 = *(u16 *)(cells + 3);
    prim->b2 = *(u16 *)(cells + 3);
    prim->r3 = *(u16 *)(cells + 3);
    prim->g3 = *(u16 *)(cells + 3);
    prim->b3 = *(u16 *)(cells + 3);
    GpuAppendPrim(5, 0x24);
  }

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0x25;
  } else {
    graph = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(5, 0xC);
}
