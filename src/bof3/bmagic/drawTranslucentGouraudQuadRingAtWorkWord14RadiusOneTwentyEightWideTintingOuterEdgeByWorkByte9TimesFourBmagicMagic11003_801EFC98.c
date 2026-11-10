#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern s16 D_1F800002; /* @source 0x1F800002 @kind data */
extern s16 D_1F80001C; /* @source 0x1F80001C @kind data */

s32 func_801782FC(s32 phase);
s32 func_801783C8(s32 phase);
s32 GetGraphType(void);

/* @source 0x801EFC98
 * @behavior Draws a translucent Gouraud quad ring in sixty-four steps of
 *           one sixty-fourth turn. Selects draw-mode tpage 0x125 when
 *           GetGraphType returns 1 or 2, otherwise 0x55, and links the
 *           draw-mode packet at ordering-table slot 2. Reads the work pointer
 *           once, seeds the inner radius from work word 0x14 and the outer
 *           radius from that word plus 0x80, and stores work byte 9 times four
 *           as the tint. Primes both edges at phase zero and clears all four
 *           vertex depths. Each pass carries both edges forward, samples
 *           their next points with signed products shifted right twelve,
 *           projects through RotTransPers4, and links a 0x24-byte POLY_G4 at
 *           slot 2. The inner edge has RGB 1 and the outer edge takes six
 *           separate reads of the tint halfword. Takes no arguments and
 *           returns nothing; wider work-field roles remain unknown.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudQuadRingAtWorkWord14RadiusOneTwentyEightWideTintingOuterEdgeByWorkByte9TimesFourBmagicMagic11003_801EFC98(void) {
  POLY_G4 *prim;
  SVECTOR *shape;
  SVECTOR *mid;
  u8 *work;
  s16 *radius;
  s16 *outer;
  s16 *cells;
  s32 phase;
  s32 step;
  s32 tpage;
  long depth;
  long flag;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0x125;
  } else {
    tpage = 0x55;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(2, 0xC);

  work = g_battle_work;
  radius = &D_1F800000;
  *radius = *(u32 *)(work + 0x14);
  D_1F800002 = *(u32 *)(work + 0x14) + 0x80;
  SPAD_REF(u16, 0x04u) = work[9] << 2;

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

  for (; step < 0x41; step++) {
    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    phase = step << 6;
    shape[0].vx = shape[1].vx;
    shape[0].vy = shape[1].vy;
    shape[1].vx = (func_801782FC(phase) * outer[0]) >> 12;
    shape[1].vy = (func_801783C8(phase) * outer[0]) >> 12;
    shape[2].vx = shape[3].vx;
    shape[2].vy = shape[3].vy;
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
    prim->r2 = *(u16 *)(cells + 2);
    prim->g2 = *(u16 *)(cells + 2);
    prim->b2 = *(u16 *)(cells + 2);
    prim->r3 = *(u16 *)(cells + 2);
    prim->g3 = *(u16 *)(cells + 2);
    prim->b3 = *(u16 *)(cells + 2);
    GpuAppendPrim(2, 0x24);
  }
}
