#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-record pointer cell at 0x1F800044. The body reads the cell
 * once, before the first scratchpad store, and reaches work byte 9 through that
 * one load; the spelling is the one this target's other lifts of the same cell
 * use. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Scratchpad radius halfword at 0x1F800000. The pre-loop publishes four times
 * unsigned work byte 9 there once and every later sample reads the halfword
 * back, so the cell stays one named row and the loop reaches it through its own
 * pointer local: the original keeps the pre-loop base and the loop base as two
 * live values, carrying the second as a copy of the first at loop entry. */
extern s16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Third of the three consecutive SVECTORs this body projects, at scratchpad
 * 0x1F800024 (the earlier two sit at 0x1F800014 and 0x1F80001C). The pre-loop
 * seeds this vertex, and the loop reaches the whole triplet from it, one vertex
 * back for the middle vertex and two back for the origin vertex. */
extern SVECTOR D_1F800024; /* @source 0x1F800024 @kind data */

/* Shared fixed-point samplers of the main executable, declared per caller as
 * the bmagic fan siblings do; both take a 12-bit phase and return the sample
 * the `>> 12` products below divide by. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared primitive seed helper that sits between the cursor read and
 * SetSemiTrans, the position the bmagic fan siblings of other overlays give
 * their own seed helper. */
void func_8017A97C(void *arg0);

/* Shared graphic-type query feeding the draw-mode texture page argument. */
s32 GetGraphType(void);

/* @source 0x801F13D4
 * @behavior Projected draw step of this overlay. It picks a draw-mode texture
 *           page, 0x125 while GetGraphType answers 1 or 2 and 0x55 otherwise,
 *           calling the query once per test with no cached result, arms
 *           SetDrawMode on the primitive cursor g_PrimCursor with dfe 0, dtd 1
 *           and a null RECT, and links that 0xC-byte packet under
 *           ordering-table slot 5. It then publishes a radius of four times
 *           unsigned work byte 9 at scratchpad halfword 0x1F800000 and seeds
 *           the last of three SVECTORs at scratchpad 0x1F800024 from the phase
 *           zero samples, scaled by that radius and shifted right by twelve.
 *           The loop emits eight translucent Gouraud triangles, one per phase
 *           step 0x200 through 0x1000: it stamps the cursor as a POLY_G3, marks
 *           it semi transparent with 1, moves the previous rim point into the
 *           middle vertex, resets the origin vertex and the depth of all three,
 *           samples the next rim point from the phase and the same radius, and
 *           projects the three vertices with RotTransPers3 straight into the
 *           packet packed coordinate words at offsets 8, 0x10 and 0x18. Vertex
 *           zero carries the grey 0x80 and both rim vertices carry 1, and each
 *           0x1C-byte packet is linked under slot 5. A second texture-page pick,
 *           0x25 for both graph types and 0x15 otherwise, closes the run with
 *           the same draw-mode packet and a 0xC-byte link under slot 5. It
 *           reads no incoming register, so it takes no arguments and returns
 *           nothing; the 0x48-byte frame keeps $s0 to $s6 and $ra, and the
 *           scratchpad triplet stays in $s2 across the eight passes. Two
 *           payload `jal` words name this span, at 0x801F0D28 in
 *           dispatchWorkByte2HandlerThenInvokeSharedServiceAndRunSubStepBelow4AndPopMatrixWhenWorkByte9SetBmagicMagic01500_801F0C30,
 *           directly before its GTE matrix pop, and at 0x801F043C in
 *           func_801F0334. The 0x25C-byte span ends exactly at the four-word
 *           handler table 0x801F1630. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawProjectedTranslucentFanOfEightGouraudTrianglesFromOriginAtRadiusFourTimesWorkByte9BmagicMagic01500_801F13D4(
    void) {
  POLY_G3 *prim;
  u8 *work;
  SVECTOR *verts;
  SVECTOR *third;
  s16 *radius;
  s16 *scale;
  s32 graph;
  s32 step;
  u16 x;
  u16 y;
  long p;
  long flag;

  if (GetGraphType() == 1) {
    graph = 0x125;
  } else if (GetGraphType() == 2) {
    graph = 0x125;
  } else {
    graph = 0x55;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(5, 0xC);

  work = D_1F800044;
  radius = &D_1F800000;
  *radius = work[9] * 4;
  third = &D_1F800024;
  third->vx = (func_801782FC(0) * *radius) >> 12;
  third->vy = (func_801783C8(0) * *radius) >> 12;
  step = 1;
  verts = third - 2;
  scale = radius;
  do {
    prim = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);
    x = verts[2].vx;
    y = verts[2].vy;
    verts[0].vx = 0;
    verts[0].vy = 0;
    verts[1].vx = x;
    verts[1].vy = y;
    verts[2].vx = (func_801782FC(step << 9) * *scale) >> 12;
    verts[2].vy = (func_801783C8(step << 9) * *scale) >> 12;
    verts[2].vz = 0;
    verts[1].vz = 0;
    verts[0].vz = 0;
    RotTransPers3(&verts[0], &verts[1], &verts[2], (long *)&prim->x0,
                  (long *)&prim->x1, (long *)&prim->x2, &p, &flag);
    prim->r0 = 0x80;
    prim->g0 = 0x80;
    prim->b0 = 0x80;
    prim->r1 = 1;
    prim->g1 = 1;
    prim->b1 = 1;
    prim->r2 = 1;
    prim->g2 = 1;
    prim->b2 = 1;
    step++;
    GpuAppendPrim(5, 0x1C);
  } while (step < 9);

  if (GetGraphType() == 1) {
    graph = 0x25;
  } else if (GetGraphType() == 2) {
    graph = 0x25;
  } else {
    graph = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(5, 0xC);
}
