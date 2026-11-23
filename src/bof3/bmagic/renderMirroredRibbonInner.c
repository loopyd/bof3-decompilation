#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, the cell this target's
 * exact lifts bind as g_battle_work. This body reads the cell once before the
 * loop and reaches every work field through that one load: unsigned byte 0xB
 * twice, unsigned byte 9 three times and the screen halfword pair 0x2E/0x30.
 * Each repeat is a fresh load because the scratchpad stores between them may
 * alias the object. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad ribbon state at 0x1F800000. The two radii are the signed
 * halfwords at 0x00 and 0x02 and the tint triple is the unsigned halfwords at
 * 0x0A, 0x0C and 0x0E. The base is an array because the original treats it as
 * a pointer value: it forms the address once for the pre-loop inner-radius
 * store, reaches the loop through a copy of that value in a saved register and
 * reads both radii and all three tint cells at their own constant offsets.
 * Only the remaining four cells are stored through their own addresses. */
extern u16 D_1F800000[]; /* @source 0x1F800000 @kind data */
extern u16 D_1F800002; /* @source 0x1F800002 @kind data */
extern u16 D_1F80000A; /* @source 0x1F80000A @kind data */
extern u16 D_1F80000C; /* @source 0x1F80000C @kind data */
extern u16 D_1F80000E; /* @source 0x1F80000E @kind data */

/* Shared rotator of the main executable at 0x801782FC: it takes a 12-bit angle
 * and returns its fixed-point sample in the scale the `>> 12` products below
 * divide by, the same role the exact bmagic ribbon siblings give it. The
 * address is outside this payload and this target's generated
 * undefined_funcs_auto.txt already binds it. */
s32 func_801782FC(s32 angle);

/* Shared graphic-type query that selects the draw-mode texture page. */
s32 GetGraphType(void);

/* @source 0x801F07A8
 * @behavior Overlay drawing routine of this payload with one caller and no
 *           pointer word naming it: the exact work-byte-2 handler
 *           callDrawTwiceActive
 *           reaches this address with two bare jal instructions from the arm it
 *           takes when work byte 0 and work byte 2 are both non-zero, and this
 *           body reads no incoming register, so it takes no arguments and
 *           returns nothing.
 *           It selects the draw-mode texture page, 0xA5 when GetGraphType
 *           returns 1 or 2 and 0x35 otherwise, arms SetDrawMode on the
 *           primitive cursor g_PrimCursor with dfe 0, dtd 1 and a null RECT,
 *           and links that 0xC-byte packet under ordering-table slot 2.
 *           It then publishes the ribbon state in scratchpad from the work
 *           object: the inner radius at 0x1F800000 is work byte 0xB, the outer
 *           radius at 0x1F800002 is twice that, and the tint triple is work
 *           byte 9 scaled by the coefficients 15, 8 and 8 at 0x1F80000A,
 *           0x1F80000C and 0x1F80000E.
 *           It emits thirty-two passes, each four translucent Gouraud quads of
 *           0x24 bytes into the same ordering-table slot 2. Pass k works at the
 *           12-bit phases k*0x40 and (k+1)*0x40 and at the column x, whose two
 *           edges are x and x+0x10 and which advances by 0x10 per pass; every
 *           corner offset is the 0x801782FC sample of the phase multiplied by
 *           a published radius and shifted right 12, added to or subtracted
 *           from the screen pair's y halfword 0x30.
 *           The first and third quads of a pass lie above and below the centre
 *           line: from it out to the inner radius times the pass's two samples,
 *           tinted with the work-byte-9 triple at the centre-line pair and with
 *           work byte 9 times 15 in all three channels at the outer pair. The
 *           second and fourth quads continue outward from the inner radius to
 *           the outer radius times the same two samples, colour 1 at the outer
 *           pair and the tint triple at the inner one, so one pass paints a
 *           ribbon mirrored above and below the centre line. The pass advances
 *           the column and the phase, and the loop closes after thirty-two
 *           passes over half a turn of sixty-fourth-turn steps.
 *           No byte-identical copy exists: every one of its call targets
 *           resolves in the shared main executable and its only data references
 *           are the shared g_PrimCursor cell and the scratchpad, so the span
 *           was searched word for word across every extracted payload under
 *           out/binaries and holds only at its own offset 0x1BA8 of this
 *           overlay. Statement order, operand forms and declaration style
 *           follow the exact bmagic emitters of this family,
 *           drawTranslucentRotatedQuadAtScreenCenterScaledAndTintedByWorkByte9BmagicMagic06000_801F0428
 *           for the draw-mode and primitive-seeding idiom and
 *           drawSixteenTranslucentGouraudFanTrianglesWithWorkByteBRadiusAndByte9TintBmagicMagic09303_801EF85C
 *           for the scratchpad cell, radius and tint-weight idiom.
 *           The 0x40-byte frame keeps $ra, $fp, $s7 to $s0 and the outgoing
 *           fifth draw-mode argument at 0x10; the counter, the two column edges,
 *           the two phases, the phase-plus-one, the cell base, the centre pair
 *           and the colour 1 stay in the saved registers across the calls.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderMirroredRibbonInner(
    void) {
  POLY_G4 *prim;
  u8 *work;
  u16 *cells;
  s32 graph;
  u16 x;
  u16 cy;
  s32 i;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0xA5;
  } else {
    graph = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(2, 0xC);

  work = g_battle_work;
  i = 0;
  D_1F800000[0] = work[0xB];
  D_1F800002 = work[0xB] << 1;
  D_1F80000A = work[9] * 15;
  D_1F80000C = work[9] * 8;
  D_1F80000E = work[9] * 8;
  cells = D_1F800000;
  x = *(u16 *)(work + 0x2E) - 0x100;
  cy = *(u16 *)(work + 0x30);

  do {
    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);

    prim->x0 = x;
    prim->y0 = cy + ((func_801782FC((u8)i << 6) * *(s16 *)cells) >> 12);
    prim->x2 = x;
    prim->y2 = cy;
    prim->x1 = x + 0x10;
    prim->y1 = cy + ((func_801782FC(((u8)i + 1) << 6) * *(s16 *)cells) >> 12);
    prim->x3 = x + 0x10;
    prim->y3 = cy;
    prim->r0 = cells[5];
    prim->g0 = cells[6];
    prim->b0 = cells[7];
    prim->r1 = cells[5];
    prim->g1 = cells[6];
    prim->b1 = cells[7];
    prim->r2 = cells[5];
    prim->g2 = cells[5];
    prim->b2 = cells[5];
    prim->r3 = cells[5];
    prim->g3 = cells[5];
    prim->b3 = cells[5];
    GpuAppendPrim(2, 0x24);

    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);

    prim->x0 = x;
    prim->y0 = cy + ((func_801782FC((u8)i << 6) * *((s16 *)cells + 1)) >> 12);
    prim->x2 = x;
    prim->y2 = cy + ((func_801782FC((u8)i << 6) * *(s16 *)cells) >> 12);
    prim->x1 = x + 0x10;
    prim->y1 = cy + ((func_801782FC(((u8)i + 1) << 6) * *((s16 *)cells + 1)) >> 12);
    prim->x3 = x + 0x10;
    prim->y3 = cy + ((func_801782FC(((u8)i + 1) << 6) * *(s16 *)cells) >> 12);
    prim->r0 = 1;
    prim->g0 = 1;
    prim->b0 = 1;
    prim->r1 = 1;
    prim->g1 = 1;
    prim->b1 = 1;
    prim->r2 = cells[5];
    prim->g2 = cells[6];
    prim->b2 = cells[7];
    prim->r3 = cells[5];
    prim->g3 = cells[6];
    prim->b3 = cells[7];
    GpuAppendPrim(2, 0x24);

    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);

    prim->x0 = x;
    prim->y0 = cy - ((func_801782FC((u8)i << 6) * *(s16 *)cells) >> 12);
    prim->x2 = x;
    prim->y2 = cy;
    prim->x1 = x + 0x10;
    prim->y1 = cy - ((func_801782FC(((u8)i + 1) << 6) * *(s16 *)cells) >> 12);
    prim->x3 = x + 0x10;
    prim->y3 = cy;
    prim->r0 = cells[5];
    prim->g0 = cells[6];
    prim->b0 = cells[7];
    prim->r1 = cells[5];
    prim->g1 = cells[6];
    prim->b1 = cells[7];
    prim->r2 = cells[5];
    prim->g2 = cells[5];
    prim->b2 = cells[5];
    prim->r3 = cells[5];
    prim->g3 = cells[5];
    prim->b3 = cells[5];
    GpuAppendPrim(2, 0x24);

    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    SetSemiTrans(prim, 1);

    prim->x0 = x;
    prim->y0 = cy - ((func_801782FC((u8)i << 6) * *((s16 *)cells + 1)) >> 12);
    prim->x2 = x;
    prim->y2 = cy - ((func_801782FC((u8)i << 6) * *(s16 *)cells) >> 12);
    prim->x1 = x + 0x10;
    prim->y1 = cy - ((func_801782FC(((u8)i + 1) << 6) * *((s16 *)cells + 1)) >> 12);
    prim->x3 = x + 0x10;
    prim->y3 = cy - ((func_801782FC(((u8)i + 1) << 6) * *(s16 *)cells) >> 12);
    prim->r0 = 1;
    prim->g0 = 1;
    prim->b0 = 1;
    prim->r1 = 1;
    prim->g1 = 1;
    prim->b1 = 1;
    prim->r2 = cells[5];
    prim->g2 = cells[6];
    prim->b2 = cells[7];
    prim->r3 = cells[5];
    prim->g3 = cells[6];
    prim->b3 = cells[7];
    GpuAppendPrim(2, 0x24);

    i++;
    x = x + 0x10;
  } while ((u8)i < 0x20);
}
