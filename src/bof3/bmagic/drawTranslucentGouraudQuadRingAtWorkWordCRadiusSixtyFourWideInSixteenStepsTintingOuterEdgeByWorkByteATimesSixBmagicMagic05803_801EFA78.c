#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target
 * map. The original reads the projection word 0x0C twice in a row, once for
 * each radius cell, and the tint byte 0x0A once before the frame is created,
 * so the tint load comes first and every projection read keeps its own load. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* The two radius halfwords of this body. The original materialises the outer
 * cell 0x1F800000 into a saved register that the pass loop reuses for both
 * radius reads and the tint word read, while the inner cell 0x1F800002 is
 * reached through its own literal address, so the two cells are separated
 * below. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern s16 D_1F800002; /* @source 0x1F800002 @kind data */

/* Outer-vertex tint halfword at 0x1F800004, work byte 0x0A times six, which
 * the pass loop reads once per outer colour byte because those byte stores may
 * alias it. */
extern u16 D_1F800004; /* @source 0x1F800004 @kind data */

/* Quad vertex cells. The original materialises the quad base 0x1F800014 once
 * in the loop preheader and reaches all eight halfwords through that register,
 * so the four z clears below are register displacements and not literals. */
extern s16 D_1F800014; /* @source 0x1F800014 @kind data */

/* Primed outer and inner edge halfwords: the first vertex of each pair takes
 * the sine sample and the second the cosine sample of turn zero, stored
 * through the cells own literal addresses. */
extern s16 D_1F80001C; /* @source 0x1F80001C @kind data */
extern s16 D_1F80001E; /* @source 0x1F80001E @kind data */
extern s16 D_1F80002C; /* @source 0x1F80002C @kind data */
extern s16 D_1F80002E; /* @source 0x1F80002E @kind data */

/* Shared rotator pair at 0x801782FC and 0x801783C8: the sine and cosine
 * samples of a 12-bit turn, which the `>> 12` products below divide out. Every
 * call here takes a phase in the same 4096-step scale, the primed vertices its
 * turn zero and each pass its step shifted left eight. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* @source 0x801EFA78
 * @behavior Translucent Gouraud quad-ring emitter of this overlay step group,
 *           the narrower and coarser companion of the one at 0x801EF74C. The
 *           step handler func_801EEF74 calls it between the work-record
 *           transform it installs through func_801EF69C and its PopMatrix, so
 *           the ring is drawn under that matrix. It first scales a tint
 *           halfword into the cell 0x1F800004 from the work object byte at
 *           0x0A times six, then seeds two radius halfwords from the work
 *           object word at 0x0C: the outer cell 0x1F800000 takes that word plus
 *           0x40 and the inner cell 0x1F800002 takes it unchanged, so the ring
 *           is 0x40 wide. It primes the outer edge 0x1F80001C and the inner
 *           edge 0x1F80002C from the rotator pair at 0x801782FC/0x801783C8,
 *           turn zero scaled by their own radius shifted right twelve, then
 *           picks the draw-mode tpage argument, 0xA5 when GetGraphType returns
 *           1 or 2 (two calls, the second only when the first answers
 *           something else) and 0x35 otherwise, seeds SetDrawMode at
 *           g_PrimCursor with dfe 0, dtd 1 and a null RECT, and links a
 *           0xC-byte packet at output slot 5. Each of the sixteen passes then
 *           takes the cursor primitive, builds it through SetPolyG4, marks it
 *           semi-transparent, moves the primed outer edge into the first vertex
 *           and the primed inner edge into the third, rebuilds the second and
 *           fourth from four more rotator samples at the step shifted left
 *           eight (one sixteenth of a turn), clears the four z halfwords, and
 *           projects the quad through RotTransPers4 with the primitive vertex
 *           pairs at 0x08, 0x10, 0x18 and 0x20 as its screen outputs and two
 *           frame words as the depth and flag results. The two outer vertices
 *           take the low byte of the tint halfword 0x1F800004 in all three
 *           channels and the two inner ones take 1. Each pass closes with one
 *           0x24-byte POLY_G4 packet linked at the same output slot. The last
 *           pass ends on turn zero again, so the quad closes the ring. Once the
 *           ring is closed the body re-runs the draw-mode selection with a
 *           second pair of tpage values, 0x25 when GetGraphType returns 1 or 2
 *           and 0x15 otherwise, re-seeds SetDrawMode at g_PrimCursor with the
 *           same dfe 0, dtd 1 and null RECT, and links one more 0xC-byte packet
 *           at output slot 5. Frame 0x50 keeps $ra, all six saved registers, the
 *           six outgoing stack words of RotTransPers4 and its two results.
 *           Takes no arguments and returns nothing; the body writes nothing
 *           outside the scratchpad and the primitives it emits.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudQuadRingAtWorkWordCRadiusSixtyFourWideInSixteenStepsTintingOuterEdgeByWorkByteATimesSixBmagicMagic05803_801EFA78(
    void) {
  POLY_G4 *prim;
  SVECTOR *shape;
  s16 *radii;
  s32 phase;
  s32 step;
  s32 tpage;
  long depth;
  long flag;

  D_1F800004 = g_battle_work[0xA] * 6;

  D_1F800000 = *(u32 *)(g_battle_work + 0xC) + 0x40;
  D_1F800002 = *(u32 *)(g_battle_work + 0xC);

  D_1F80001C = (func_801782FC(0) * D_1F800000) >> 12;
  D_1F80001E = (func_801783C8(0) * D_1F800000) >> 12;
  D_1F80002C = (func_801782FC(0) * D_1F800002) >> 12;
  D_1F80002E = (func_801783C8(0) * D_1F800002) >> 12;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(5, 0xC);

  step = 1;
  shape = (SVECTOR *)&D_1F800014;
  radii = &D_1F800000;

  for (; step < 0x11; step++) {
    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    phase = step << 8;
    shape[0].vx = shape[1].vx;
    shape[0].vy = shape[1].vy;
    shape[0].vz = 0;
    shape[1].vx = (func_801782FC(phase) * radii[0]) >> 12;
    shape[1].vy = (func_801783C8(phase) * radii[0]) >> 12;
    shape[1].vz = 0;
    shape[2].vx = shape[3].vx;
    shape[2].vy = shape[3].vy;
    shape[2].vz = 0;
    shape[3].vx = (func_801782FC(phase) * radii[1]) >> 12;
    shape[3].vy = (func_801783C8(phase) * radii[1]) >> 12;
    shape[3].vz = 0;
    prim->r0 = *(u16 *)(radii + 2);
    prim->g0 = *(u16 *)(radii + 2);
    prim->b0 = *(u16 *)(radii + 2);
    prim->r1 = *(u16 *)(radii + 2);
    prim->g1 = *(u16 *)(radii + 2);
    prim->b1 = *(u16 *)(radii + 2);
    prim->r2 = 1;
    prim->g2 = 1;
    prim->b2 = 1;
    prim->r3 = 1;
    prim->g3 = 1;
    prim->b3 = 1;
    RotTransPers4(shape, shape + 1, shape + 2, shape + 3, (long *)&prim->x0,
                  (long *)&prim->x1, (long *)&prim->x2, (long *)&prim->x3,
                  &depth, &flag);
    GpuAppendPrim(5, 0x24);
  }

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0x25;
  } else {
    tpage = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(5, 0xC);
}
