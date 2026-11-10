#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, the name this overlay's
 * exact lifts use for it. The body reads the cell once and takes three fields
 * of the record through that one pointer: byte 0xB twice, for the leading edge
 * radius and its double, and byte 0x5D once for the flat tint. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad quad cells. The original materialises the leading-edge base
 * 0x1F800000 into a saved register and reaches its byte-doubled radius cell
 * 0x1F800002 as +2 of it, so both are one pointer below. The four vertex
 * halfwords 0x1F80001C, 0x1F80002C and the tint halfword 0x1F80000A keep their
 * own absolute stores, and the four cleared z halfwords 0x1F800018,
 * 0x1F800020, 0x1F800028 and 0x1F800030 are written through absolute
 * addresses only. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern s16 D_1F800002; /* @source 0x1F800002 @kind data */
extern u16 D_1F80000A; /* @source 0x1F80000A @kind data */
extern s16 D_1F800018; /* @source 0x1F800018 @kind data */
extern s16 D_1F80001C; /* @source 0x1F80001C @kind data */
extern s16 D_1F80001E; /* @source 0x1F80001E @kind data */
extern s16 D_1F800020; /* @source 0x1F800020 @kind data */
extern s16 D_1F800028; /* @source 0x1F800028 @kind data */
extern s16 D_1F80002C; /* @source 0x1F80002C @kind data */
extern s16 D_1F80002E; /* @source 0x1F80002E @kind data */
extern s16 D_1F800030; /* @source 0x1F800030 @kind data */

/* Shared rotator pair at 0x801782FC and 0x801783C8: the sine and cosine
 * samples of a 12-bit turn in the fixed-point scale the `>> 12` products below
 * divide out. Both are called with the angle argument only, seeded with zero in
 * the entry block and with the pass phase shifted left six in the loop. */
extern s32 func_801782FC(s32 angle); /* @source 0x801782FC @kind function */
extern s32 func_801783C8(s32 angle); /* @source 0x801783C8 @kind function */

/* @source 0x801EFC40
 * @behavior Last draw helper of this overlay's work-byte-2 handler group: its
 *           only in-image reference is the jal at 0x801EF048 inside the
 *           handler at 0x801EEFBC, taken while the work object's byte 0 is odd
 *           and its byte 2 is non-zero, between func_801EF9F0 and PopMatrix.
 *           The body reads no incoming register, so it takes no arguments and
 *           returns nothing.
 *           It picks the draw-mode texture page, 0x125 when GetGraphType
 *           returns 1 or 2 and 0x55 otherwise, calling the query once per test
 *           with no cached answer, arms SetDrawMode on the primitive cursor
 *           g_PrimCursor with dfe 0, dtd 1 and a null RECT, and links the
 *           0xC-byte packet at output slot 5.
 *           It then reads the scratchpad work object once: the unsigned byte
 *           0xB becomes the 0x1F800000 radius and its double the 0x1F800002
 *           radius, and the signed byte 0x5D times four becomes the tint
 *           halfword 0x1F80000A. The quad is primed at turn zero: the second
 *           vertex takes the rotator samples times the doubled radius shifted
 *           right twelve, the fourth vertex the same samples times the single
 *           radius, and the four z halfwords are cleared.
 *           Each of the sixty-four passes then takes the cursor primitive,
 *           seeds it through SetPolyG4, marks it semi-transparent, moves the
 *           primed doubled-radius edge into the first vertex and the
 *           primed single-radius edge into the third, rebuilds the second and
 *           fourth from four more rotator samples at the step shifted left six
 *           (one sixty-fourth of a turn), and projects the quad through
 *           RotTransPers4 with the primitive vertex pairs at 0x08, 0x10, 0x18
 *           and 0x20 as its screen outputs and two frame words as the depth and
 *           flag results. The two doubled-radius vertices take the constant 1
 *           in every channel and the two single-radius vertices take the tint
 *           in all three, so each translucent band grades from white at the
 *           outer edge to the tint grey at the inner edge. Each pass closes
 *           with one 0x24-byte POLY_G4 packet linked through GpuAppendPrim at
 *           the same output slot. The last pass ends on the prime turn again,
 *           so its far edges land on the primed leading edges and the
 *           sixty-four quads close the full ring.
 *           The 0x50-byte frame keeps $ra, all six $s registers, the six
 *           outgoing stack words of RotTransPers4 and its two results, and the
 *           body writes nothing outside the scratchpad and the primitive it
 *           emits. The roles of work-object byte 0x5D beyond the tint scale and
 *           of byte 0xB beyond the two radii are not established, so they stay
 *           offset-anchored and the scratchpad cells are named by address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudQuadRingBetweenWorkByteBAndTwiceWorkByteBInSixtyFourStepsTintingInnerEdgeByWorkByte5dTimesFourBmagicMagic22003_801EFC40(
    void) {
  POLY_G4 *prim;
  SVECTOR *shape;
  SVECTOR *mid;
  s16 *near;
  s16 *far;
  u16 *tint;
  s32 phase;
  s32 step;
  s32 tpage;
  s32 shade;
  long depth;
  long flag;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0x125;
  } else {
    tpage = 0x55;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(5, 0xC);

  near = &D_1F800000;
  *near = g_battle_work[0xB];
  D_1F800002 = g_battle_work[0xB] << 1;
  shade = *(s8 *)(g_battle_work + 0x5D);
  D_1F80000A = shade << 2;

  mid = (SVECTOR *)&D_1F80001C;
  mid->vx = (func_801782FC(0) * D_1F800002) >> 12;
  D_1F80001E = (func_801783C8(0) * D_1F800002) >> 12;
  D_1F80002C = (func_801782FC(0) * *near) >> 12;
  D_1F80002E = (func_801783C8(0) * *near) >> 12;

  step = 1;
  shape = mid - 1;
  far = near + 1;
  tint = (u16 *)near;
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
    prim->r2 = tint[5];
    prim->g2 = tint[5];
    prim->b2 = tint[5];
    prim->r3 = tint[5];
    prim->g3 = tint[5];
    prim->b3 = tint[5];
    GpuAppendPrim(5, 0x24);
  }
}
