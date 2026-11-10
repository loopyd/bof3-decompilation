#include "bof3/bof3.h"
#include "gpu/prim.h"
#include <stdlib.h>

extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern s16 D_1F800002; /* @source 0x1F800002 @kind data */
extern s16 D_1F80001C; /* @source 0x1F80001C @kind data */

extern s32 func_801782FC(s32 phase);
extern s32 func_801783C8(s32 phase);
extern s32 GetGraphType(void);

/* @source 0x801EF438
 * @behavior Draws a translucent Gouraud quad ring in sixteen steps. Selects
 *           draw-mode tpage 0xA5 for graph types 1 or 2, otherwise 0x35, and
 *           links its packet at slot 5. Seeds scratchpad radii with twice work
 *           byte A and three times that byte plus (rand() & 3). Seeds RGB
 *           halfwords with work byte 9 times 15, 4 and 15. Primes both edges
 *           at phase zero, clears their four depth halfwords, then carries
 *           each edge forward and samples the next edge at step << 8 using
 *           signed products shifted right twelve. Projects each quad through
 *           RotTransPers4; its first two vertices take RGB 1 and the other
 *           two take fresh reads of the tint halfwords. Links each 0x24-byte
 *           POLY_G4 at slot 5. Takes no arguments and returns nothing. Work
 *           field roles beyond these radius and tint uses remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSixteenStepJitteredGouraudRingWithWorkByteARadiiAndWorkByte9TintBmagicMagic12603_801EF438(void) {
  POLY_G4 *prim;
  SVECTOR *shape;
  SVECTOR *mid;
  u8 *work;
  s16 *radius;
  s16 *outer;
  s16 *cells;
  s32 jitter;
  s32 phase;
  s32 step;
  s32 tpage;
  long depth;
  long flag;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(5, 0xC);

  radius = &D_1F800000;
  *radius = D_1F800044[0xA] << 1;
  jitter = rand();
  work = D_1F800044;
  D_1F800002 = work[0xA] * 3 + (jitter & 3);
  SPAD_REF(u16, 0x06u) = work[9] * 15;
  SPAD_REF(u16, 0x08u) = work[9] << 2;
  SPAD_REF(u16, 0x0Au) = work[9] * 15;

  mid = (SVECTOR *)&D_1F80001C;
  mid->vx = (func_801782FC(0) * D_1F800002) >> 12;
  SPAD_REF(s16, 0x1Eu) = (func_801783C8(0) * D_1F800002) >> 12;
  SPAD_REF(s16, 0x2Cu) = (func_801782FC(0) * *radius) >> 12;
  SPAD_REF(s16, 0x2Eu) = (func_801783C8(0) * *radius) >> 12;

  step = 1;
  shape = mid - 1;
  outer = radius + 1;
  cells = radius;
  SPAD_REF(s16, 0x30u) = 0;
  SPAD_REF(s16, 0x28u) = 0;
  SPAD_REF(s16, 0x20u) = 0;
  SPAD_REF(s16, 0x18u) = 0;

  for (; step < 0x11; step++) {
    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    phase = step << 8;
    shape[0].vx = shape[1].vx;
    shape[0].vy = shape[1].vy;
    shape[2].vx = shape[3].vx;
    shape[2].vy = shape[3].vy;
    shape[1].vx = (func_801782FC(phase) * outer[0]) >> 12;
    shape[1].vy = (func_801783C8(phase) * outer[0]) >> 12;
    shape[3].vx = (func_801782FC(phase) * outer[-1]) >> 12;
    shape[3].vy = (func_801783C8(phase) * outer[-1]) >> 12;
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
    prim->g2 = *(u16 *)(cells + 4);
    prim->b2 = *(u16 *)(cells + 5);
    prim->r3 = *(u16 *)(cells + 3);
    prim->g3 = *(u16 *)(cells + 4);
    prim->b3 = *(u16 *)(cells + 5);
    GpuAppendPrim(5, 0x24);
  }
}
