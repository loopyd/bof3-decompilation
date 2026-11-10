#include "bof3/bof3.h"
#include "gpu/prim.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s32 D_1F800000; /* @source 0x1F800000 @kind data */
extern s32 D_1F800004; /* @source 0x1F800004 @kind data */
extern s32 D_1F800008; /* @source 0x1F800008 @kind data */
extern u16 D_1F80002C; /* @source 0x1F80002C @kind data */
extern u16 D_1F80002E; /* @source 0x1F80002E @kind data */
extern u16 D_1F800030; /* @source 0x1F800030 @kind data */
extern u8 D_801F03D4[]; /* @source 0x801F03D4 @kind data */

extern s32 func_801782FC(s32 angle); /* @source 0x801782FC */
extern s32 func_801783C8(s32 angle); /* @source 0x801783C8 */
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size); /* @source 0x80155A08 */
extern s32 GetGraphType(void);

/* @source 0x801EFAA8
 * @behavior Draws sixteen semi-transparent Gouraud quads under the current
 *           transform. Selects tpage 0xA5 for graph types 1 and 2, otherwise
 *           0x35, seeds a draw-mode packet and links it at work words 0x34
 *           and 0x38 with kind 2 and size 12. Stores work byte 4 times twelve
 *           as the far-edge grey level, and seeds scratchpad vertices from
 *           the rotator pair using the phase byte indexed by work byte 8 at
 *           0x801F03D4. Steps 1 through 16 advance the radial angle by 128,
 *           advance the selected phase modulo 32, carry the preceding edge
 *           forward and place the new edge at sine/cosine samples divided by
 *           32. The z coordinate alternates with the step modulo four, at
 *           amplitude sine(step * 128) times 64 divided by 4096; the other
 *           edge is offset by 80. Projects each quad with RotTransPers4,
 *           sets its first two vertices to grey 1 and its last two to the
 *           stored grey level, then links a kind-2, size-36 packet at the
 *           same work coordinates. Called without arguments by 0x801EF014;
 *           returns nothing. Uses scratchpad words 0, 4 and 8 and four
 *           vectors at 0x1F800014; its 72-byte frame retains five saved
 *           registers.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSixteenTranslucentGouraudQuadsWithWorkSelectedPhaseAndAlternatingZBmagicMagic01303_801EFAA8(void) {
  POLY_G4 *prim;
  u16 *shape;
  u16 *edge;
  s32 *values;
  s32 *tint;
  s32 step;
  s32 tpage;
  s32 phase;
  s32 z;
  u16 x0;
  u16 y0;
  u16 z0;
  long depth;
  long flag;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  func_80155A08(*(s32 *)(g_battle_work + 0x34),
                *(s32 *)(g_battle_work + 0x38), 2, 0xC);

  tint = &D_1F800008;
  *tint = g_battle_work[4] * 12;
  D_1F800000 = (func_801782FC(0) << 6) >> 12;
  D_1F800004 = D_801F03D4[g_battle_work[8]] << 7;
  edge = &D_1F80002C;
  *edge = (u32)func_801782FC(D_1F800004) >> 5;
  D_1F80002E = (u32)func_801783C8(D_1F800004) >> 5;
  D_1F800030 = (func_801782FC(0) * D_1F800000) >> 12;

  step = 1;
  values = tint - 2;
  shape = edge - 12;
  for (; step < 0x11; step++) {
    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);
    values[0] = (func_801782FC(step << 7) << 6) >> 12;
    phase = D_801F03D4[g_battle_work[8]];
    x0 = shape[12];
    y0 = shape[13];
    z0 = shape[14];
    shape[0] = x0;
    shape[1] = y0;
    shape[2] = z0 + 0x50;
    shape[8] = x0;
    shape[9] = y0;
    shape[10] = z0;
    values[1] = ((phase + step) & 0x1F) << 7;
    shape[12] = (u32)func_801782FC(values[1]) >> 5;
    shape[13] = (u32)func_801783C8(values[1]) >> 5;
    z = (func_801782FC((step & 3) << 10) * values[0]) >> 12;
    shape[4] = shape[12];
    shape[5] = shape[13];
    shape[14] = z;
    shape[6] = z + 0x50;
    RotTransPers4((SVECTOR *)shape, (SVECTOR *)(shape + 4),
                  (SVECTOR *)(shape + 8), (SVECTOR *)(shape + 12),
                  (long *)&prim->x0, (long *)&prim->x1, (long *)&prim->x2,
                  (long *)&prim->x3, &depth, &flag);
    prim->r0 = 1;
    prim->g0 = 1;
    prim->b0 = 1;
    prim->r1 = 1;
    prim->g1 = 1;
    prim->b1 = 1;
    prim->r2 = values[2];
    prim->g2 = values[2];
    prim->b2 = values[2];
    prim->r3 = values[2];
    prim->g3 = values[2];
    prim->b3 = values[2];
    func_80155A08(*(s32 *)(g_battle_work + 0x34),
                  *(s32 *)(g_battle_work + 0x38), 2, 0x24);
  }
}
