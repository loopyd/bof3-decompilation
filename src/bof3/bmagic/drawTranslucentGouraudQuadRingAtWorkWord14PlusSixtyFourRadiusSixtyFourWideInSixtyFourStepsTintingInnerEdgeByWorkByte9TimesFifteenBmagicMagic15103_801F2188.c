#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map. The original reads the projection word 0x14 through it twice in a row,
 * once for each of the two radius cells, and the work byte 0x09 once for the
 * tint, so every read keeps its own load. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* The two radius halfwords of this body. The original materialises the inner
 * cell 0x1F800002 into a saved register, biases the projection low halfword by
 * 0x40 into it, and materialises the outer cell 0x1F800004 only as the literal
 * store address, then derives the outer pointer from the inner one with one
 * `addiu 2` in the loop preheader, so the two cells are addressed through one
 * pointer below. */
extern s16 D_1F800002; /* @source 0x1F800002 @kind data */
extern s16 D_1F800004; /* @source 0x1F800004 @kind data */

/* Tint halfword at 0x1F80000A, work byte 0x09 times fifteen. The original
 * stores it through its literal address and reads it back in the pass loop
 * through the outer-radius pointer at displacement six, once per vertex colour
 * byte, because those byte stores may alias it. */
extern u16 D_1F80000A; /* @source 0x1F80000A @kind data */

/* Quad vertex cells. The original holds the second vertex x at 0x1F80001C in a
 * saved register, primes it there, and derives the quad base 0x1F800014 from
 * that register with one `addiu -8` in the loop preheader. The remaining primed
 * halfwords are reached through their own literal addresses. */
extern s16 D_1F80001C; /* @source 0x1F80001C @kind data */
extern s16 D_1F800018; /* @source 0x1F800018 @kind data */
extern s16 D_1F80001E; /* @source 0x1F80001E @kind data */
extern s16 D_1F800020; /* @source 0x1F800020 @kind data */
extern s16 D_1F800028; /* @source 0x1F800028 @kind data */
extern s16 D_1F80002C; /* @source 0x1F80002C @kind data */
extern s16 D_1F80002E; /* @source 0x1F80002E @kind data */
extern s16 D_1F800030; /* @source 0x1F800030 @kind data */

/* Shared rotator pair at 0x801782FC and 0x801783C8: the sine and cosine
 * samples of a 12-bit turn, which the `>> 12` products below divide out. Every
 * call here takes a phase in the same 4096-step scale, the primed vertices its
 * turn zero and each pass its step shifted left six. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* @source 0x801F2188
 * @behavior Translucent Gouraud quad-ring emitter of this overlay draw group,
 *           the second of the two quad drawers the work byte 2 handler at
 *           0x801F1C3C calls after it installs the record transform. It takes
 *           no arguments, returns nothing and is reached only through that
 *           call. It draws the band that continues the first drawer of the
 *           pair outward: it first picks the draw-mode tpage argument, 0xA5
 *           when GetGraphType returns 1 or 2 (two calls, the second only when
 *           the first answers something else) and 0x35 otherwise, seeds
 *           SetDrawMode at g_PrimCursor with dfe 0, dtd 1 and a null RECT, and
 *           links a 0xC-byte packet with GpuAppendPrim at output slot 4. It
 *           then reads the work object projection word 0x14 twice: the low
 *           halfword biased by 0x40 becomes the inner radius 0x1F800002 and
 *           the same low halfword biased by 0x80 the outer radius 0x1F800004,
 *           so the ring is 0x40 wide and sits one band outside the first
 *           drawer, sharing that drawer's outer radius as its inner edge. Work
 *           byte 0x09 times fifteen becomes the tint halfword 0x1F80000A. It
 *           primes the leading edge of the quad at 0x1F800014 from turn zero:
 *           the second vertex takes the sine and cosine samples times the outer
 *           radius shifted right twelve, the fourth vertex the same samples
 *           times the inner radius, and the four z halfwords are cleared. Each
 *           of the sixty-four passes then takes the cursor primitive, seeds it
 *           through SetPolyG4, marks it semi-transparent, moves the primed
 *           outer and inner edges into the first and third vertices, rebuilds
 *           the second and fourth from four more rotator samples at the step
 *           shifted left six (one sixty-fourth of a turn), and projects the
 *           quad through RotTransPers4 with the primitive vertex pairs at 0x08,
 *           0x10, 0x18 and 0x20 as its screen outputs and two frame words as
 *           the depth and flag results. The two inner vertices take 1 and the
 *           two outer vertices take the tint in all three channels, the
 *           inverse of the first drawer, so the pair grades from 1 at both
 *           extremes of the 0x80-wide band toward the tint at their shared
 *           radius. Each pass closes with one 0x24-byte POLY_G4 packet linked
 *           through GpuAppendPrim at the same output slot. The last pass ends
 *           on the prime turn again, so the quad closes the ring. Frame 0x50
 *           keeps $ra, all six $s registers, the six outgoing stack words of
 *           RotTransPers4 and its two results, and the body writes nothing
 *           outside the scratchpad and the primitive it emits.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudQuadRingAtWorkWord14PlusSixtyFourRadiusSixtyFourWideInSixtyFourStepsTintingInnerEdgeByWorkByte9TimesFifteenBmagicMagic15103_801F2188(
    void) {
  POLY_G4 *prim;
  SVECTOR *shape;
  SVECTOR *mid;
  s16 *near;
  s16 *far;
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
  GpuAppendPrim(4, 0xC);

  near = &D_1F800002;
  *near = *(u32 *)(g_battle_work + 0x14) + 0x40;
  D_1F800004 = *(u32 *)(g_battle_work + 0x14) + 0x80;
  D_1F80000A = g_battle_work[9] * 15;

  mid = (SVECTOR *)&D_1F80001C;
  mid->vx = (func_801782FC(0) * D_1F800004) >> 12;
  D_1F80001E = (func_801783C8(0) * D_1F800004) >> 12;
  D_1F80002C = (func_801782FC(0) * *near) >> 12;
  D_1F80002E = (func_801783C8(0) * *near) >> 12;

  step = 1;
  shape = mid - 1;
  far = near + 1;
  D_1F800030 = 0;
  D_1F800028 = 0;
  D_1F800020 = 0;
  D_1F800018 = 0;

  for (; step < 0x41; step++) {
    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    phase = step << 6;
    shape[0].vx = shape[1].vx;
    shape[0].vy = shape[1].vy;
    shape[1].vx = (func_801782FC(phase) * far[0]) >> 12;
    shape[1].vy = (func_801783C8(phase) * far[0]) >> 12;
    shape[2].vx = shape[3].vx;
    shape[2].vy = shape[3].vy;
    shape[3].vx = (func_801782FC(phase) * far[-1]) >> 12;
    shape[3].vy = (func_801783C8(phase) * far[-1]) >> 12;
    RotTransPers4(shape, shape + 1, shape + 2, shape + 3, (long *)&prim->x0,
                  (long *)&prim->x1, (long *)&prim->x2, (long *)&prim->x3,
                  &depth, &flag);
    prim->r0 = 1;
    prim->g0 = 1;
    prim->b0 = 1;
    prim->r1 = 1;
    prim->g1 = 1;
    prim->b1 = 1;
    prim->r2 = *(u16 *)(far + 3);
    prim->g2 = *(u16 *)(far + 3);
    prim->b2 = *(u16 *)(far + 3);
    prim->r3 = *(u16 *)(far + 3);
    prim->g3 = *(u16 *)(far + 3);
    prim->b3 = *(u16 *)(far + 3);
    GpuAppendPrim(4, 0x24);
  }
}
