#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work. The body loads the cell once for the tint and the two band
 * radii, then never again. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad disc cells at 0x1F800000. The halfword at 0x1F800000 is the ripple
 * radius, work-object byte 0xA shifted left four; the halfword at 0x1F800002 is
 * the band radius, work-object byte 9 shifted left four, and drops by 0x40 per
 * band. The halfword at 0x1F800004 keeps the previous band radius and
 * 0x1F800006 the current longitude phase. The three halfwords at 0x1F800008,
 * 0x1F80000A and 0x1F80000C hold the grey tint, work-object byte 0x5D shifted
 * left three, one per colour lane. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */
extern u16 D_1F800002; /* @source 0x1F800002 @kind data */
extern s16 D_1F800008; /* @source 0x1F800008 @kind data */
extern s16 D_1F80000A; /* @source 0x1F80000A @kind data */
extern s16 D_1F80000C; /* @source 0x1F80000C @kind data */
extern u16 D_1F800020; /* @source 0x1F800020 @kind data */

/* The four scratchpad vectors at 0x1F800014 that RotTransPers4 projects: this
 * body reaches the first through the depth cell minus 0xC and the last through
 * the depth cell plus 0xC, and passes the two interior addresses of the second
 * and third vectors as their own symbols. */
extern SVECTOR D_1F80001C; /* @source 0x1F80001C @kind data */
extern SVECTOR D_1F800024; /* @source 0x1F800024 @kind data */

/* The texture-atlas byte lanes this target keeps after the handler pointer run
 * at 0x801F1BF4: one v coordinate per band at 0x801F1C13 and the band height
 * that the second v coordinate adds at 0x801F1C1B. */
extern u8 D_801F1C13[]; /* @source 0x801F1C13 @kind data */
extern u8 D_801F1C1B[]; /* @source 0x801F1C1B @kind data */

/* Shared phase rotators of the main executable: the 0x801782FC sample feeds
 * every x halfword and the 0x801783C8 sample every y halfword, in the fixed
 * point scale the `>> 12` products below divide out. */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */

/* Shared graphic-type query feeding the draw-mode texture-page argument. */
s32 GetGraphType(void);

/* @source 0x801F06B4
 * @behavior Draws a rippled disc as eight concentric ring bands of sixteen
 *           longitude steps, one semi-transparent textured quad per step. It
 *           selects the draw-mode texture page from GetGraphType, 0xA5 for
 *           graph types 1 and 2 and 0x35 otherwise, arms SetDrawMode on the
 *           shared primitive cursor at 0x8014598C with dfe 0, dtd 1 and a null
 *           tw, and appends a 0xC-byte packet to output list 3 through the
 *           shared append routine at 0x8014E5A0. It reads the scratchpad
 *           work-object pointer cell at 0x1F800044 once: work-object byte 0x5D
 *           shifted left three becomes the three grey tint lanes, and work
 *           bytes 0xA and 9 shifted left four seed the ripple radius 0x1F800000
 *           and the band radius 0x1F800002. Each band copies that radius into
 *           0x1F800004 and drops the band radius by 0x40, samples the phase-zero
 *           rotators against the previous radius into the last vector, copies
 *           the previous band depth 0x1F800020 into the depth of the third and
 *           fourth vectors, samples the same phase-zero rotators against the
 *           new radius into the second vector, and stores
 *           -(cos(band << 8) * ripple radius >> 12) into the depth of the first
 *           and second vectors. The sixteen steps of a band run (i & 0xF) << 8
 *           in 0x100 strides: each step publishes that phase into 0x1F800006,
 *           copies the sample holding the previous longitude into the first
 *           vector, samples both rotators at the phase against the new radius
 *           into the second vector and against the previous radius into the
 *           fourth vector, and copies the fourth vector into the third. It then
 *           stamps a POLY_FT4 at the primitive cursor through SetPolyFT4, sets
 *           semi-transparency with a fixed one, writes GetTPage(1, 1, 0x340,
 *           0x100) and GetClut(0, 0x1FA), places the quad in the 0x40-wide
 *           atlas column 0xC0 wide by the band group and the step parity and
 *           the 0x3F-wide neighbour, reads the two band rows of the atlas byte
 *           lanes for its four v coordinates, copies the three tint lanes into
 *           the red, green and blue bytes, projects the four vectors into x0 to
 *           x3 through RotTransPers4, and appends the 0x28-byte primitive to
 *           output list 3 unless the step is 10 or 11. It takes no arguments
 *           and returns nothing; the 0x58-byte frame keeps $ra, $fp and $s0 to
 *           $s7, and the two projection outputs stay in the frame at 0x28 and
 *           0x2C. Unknown work-object and cell roles stay offset-anchored.
 * @status partial
 * @match 92.47
 * @residual 270/292 original instructions; 1152 bytes against 1168. The
 *           first difference is at +0x2FC in the atlas column calculation.
 *           Reusing the third-vector local for the outer band's real rim
 *           accesses keeps its later address assignment in the inner-loop
 *           preheader; separate temporaries for the two vector copies also
 *           recover their original load/store order. The atlas column's signed
 *           division uses different temporaries. The original also stores the
 *           first v coordinate before it forms u1, copies
 *           the u value for u2, and loads the 0x801F1C1B lane after the
 *           0x801F1C13 lane of the same v expression. Compiler and flags
 *           remain unchanged.
 */
void drawSemiTransTexturedRippledDiscOfEightRingsSixteenSectorsWithRadiiFromWorkByte9AndRippleFromWorkByteATintedByWorkByte5DBmagicMagic13103_801F06B4(
    void) {
  POLY_FT4 *prim;
  SVECTOR *shape;
  SVECTOR *rim;
  SVECTOR *third;
  u16 *depth;
  s16 *tint;
  s16 *prev;
  s16 *phase;
  u8 *work;
  u8 row;
  u8 band;
  u8 i;
  s32 tpage;
  s32 u;
  s32 quarter;
  u16 vx;
  u16 vy;
  u16 next_vx;
  u16 next_vy;
  long p;
  long flag;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(3, 0xC);
  work = g_battle_work;
  tint = &D_1F800008;
  tint[0] = (s8)work[0x5D] * 8;
  D_1F80000A = (s8)work[0x5D] * 8;
  D_1F80000C = (s8)work[0x5D] * 8;
  D_1F800000 = work[0xA] << 4;
  D_1F800002 = work[9] << 4;
  depth = &D_1F800020;
  *depth = -((func_801782FC(0) * D_1F800000) >> 12);
  row = 1;
  prev = tint - 2;
  phase = tint - 1;
  rim = (SVECTOR *)(depth + 6);
  shape = (SVECTOR *)(depth - 6);
  do {
    prev[0] = *(u16 *)(prev - 1);
    prev[-1] = prev[0] - 0x40;
    third = rim;
    third[0].vx = (func_801782FC(0) * prev[0]) >> 12;
    third[0].vy = (func_801783C8(0) * prev[0]) >> 12;
    third[0].vz = third[-1].vz = ((u16 *)third)[-6];
    third[-2].vx = (func_801782FC(0) * prev[-1]) >> 12;
    third[-2].vy = (func_801783C8(0) * prev[-1]) >> 12;
    third[-2].vz = third[-3].vz = -((func_801782FC(row << 8) * prev[-2]) >> 12);
    i = 1;
    third = (SVECTOR *)&D_1F800024;
    band = row;
    do {
      phase[0] = (i & 0xF) << 8;
      vx = shape[1].vx;
      vy = shape[1].vy;
      shape[0].vx = vx;
      shape[0].vy = vy;
      shape[1].vx = (func_801782FC((i & 0xF) << 8) * phase[-2]) >> 12;
      shape[1].vy = (func_801783C8(phase[0]) * phase[-2]) >> 12;
      next_vx = shape[3].vx;
      next_vy = shape[3].vy;
      shape[2].vx = next_vx;
      shape[2].vy = next_vy;
      shape[3].vx = (func_801782FC(phase[0]) * phase[-1]) >> 12;
      shape[3].vy = (func_801783C8(phase[0]) * phase[-1]) >> 12;
      prim = (POLY_FT4 *)g_PrimCursor;
      SetPolyFT4(prim);
      SetSemiTrans(prim, 1);
      prim->tpage = GetTPage(1, 1, 0x340, 0x100);
      prim->clut = GetClut(0, 0x1FA);
      quarter = (band - 1) / 4;
      u = ((i & 1) + (quarter << 1)) << 6;
      prim->u0 = u;
      prim->v0 = D_801F1C13[band];
      prim->u1 = u + 0x3F;
      prim->v1 = D_801F1C13[band];
      prim->u2 = u;
      prim->v2 = D_801F1C1B[band] + (D_801F1C13[band] + 0xFF);
      prim->u3 = u + 0x3F;
      prim->v3 = D_801F1C1B[band] + (D_801F1C13[band] + 0xFF);
      RotTransPers4(shape, (SVECTOR *)&D_1F80001C, third, rim,
                    (long *)&prim->x0, (long *)&prim->x1, (long *)&prim->x2,
                    (long *)&prim->x3, &p, &flag);
      prim->r0 = ((u16 *)phase)[1];
      prim->g0 = ((u16 *)phase)[2];
      prim->b0 = ((u16 *)phase)[3];
      if (i != 10 && i != 11) {
        GpuAppendPrim(3, 0x28);
      }
      i++;
    } while (i < 17);
    row++;
  } while (row < 9);
}
