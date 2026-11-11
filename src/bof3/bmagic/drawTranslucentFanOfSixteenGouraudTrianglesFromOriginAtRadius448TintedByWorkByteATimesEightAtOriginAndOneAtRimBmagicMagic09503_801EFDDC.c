#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared battle work pointer in the scratchpad, bound by this target's map.
 * The reads fold into lui plus the 0x44 displacement, the form the original
 * emits. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius word at 0x1F800000. This body publishes the constant
 * 0x1C0 with a word store and re-reads the cell as a signed word after each
 * sampler call, because those calls may rewrite it. */
extern s32 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad next-sample halfword at 0x1F800024, written once before the loop
 * through a register base and carried forward as the third vertex pair. */
extern u16 D_1F800024; /* @source 0x1F800024 @kind data */

extern s32 GetGraphType(void);
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */
extern void func_8017A97C(void *primitive); /* @source 0x8017A97C */
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size); /* @source 0x80155A08 */

/* @source 0x801EFDDC
 * @behavior Draws sixteen semi-transparent Gouraud fan triangles from the
 *           origin at radius 0x1C0, one per 0x100 phase step, so the sixteen
 *           wedges close a full disc. Publishes that constant with a word
 *           store to scratchpad 0x1F800000 and seeds the first sample pair at
 *           phase zero into 0x1F800024 and 0x1F800026 from the two shared
 *           samplers scaled by the published radius and shifted right twelve.
 *           Both pre-loop reads take the radius through its own pointer; in
 *           the loop the reads go through a copied base pointer, because the
 *           phase value owns the original base register there. Each pass
 *           clears the origin vertex triplet, carries the previous sample pair
 *           into the second vertex, samples the current phase into the third
 *           vertex and zeroes that vertex's depth. It then selects the
 *           draw-mode page from GetGraphType,
 *           calling it separately per test: 0xA5 for graph type 1 or 2,
 *           otherwise 0x35. It arms SetDrawMode on the primitive cursor
 *           g_PrimCursor with dfe 0, dtd 1 and a null RECT, links that
 *           0xC-byte packet through 0x80155A08 with work words 0x34 and 0x38
 *           and kind 2, takes a fresh cursor primitive, seeds it through
 *           0x8017A97C and marks it semi-transparent. Work byte 0xA scaled by
 *           eight is published to scratchpad word 0x1F800008 and re-read into
 *           the red, green and blue bytes of the first vertex, because the
 *           primitive byte stores may alias that cell; the second and third
 *           vertex carry the value one in all three channels. That one is a
 *           preheader local, not a literal: the original materializes it once
 *           before the loop and the graph-type test compares against the same
 *           register, so the source names it and uses it in both places. The
 *           three vertices project through RotTransPers3 into the cursor
 *           primitive pairs at 0x08, 0x10 and 0x18, and the 0x1C-byte POLY_G3
 *           is linked through 0x80155A08 under the same kind 2.
 *           The four step handlers of this overlay at 0x801EEE60, 0x801EEF64,
 *           0x801EEFFC and 0x801EF070 call this with no arguments and consume
 *           no return value, so it draws with the matrix those handlers
 *           publish. The meaning of work byte 0xA and of the radius value
 *           beyond these published stores is not established, so they stay
 *           offset-anchored.
 *           Statement order, operand forms and scratchpad access forms follow
 *           the exact siblings at radius 240 of bmagic/magic088/03 and at
 *           radius 256 plus work byte B of bmagic/magic093/03, whose bodies
 *           differ in this function's radius cell, per-pass draw-mode packet
 *           and work-byte tint. Shape evidence, not clone proof: no payload
 *           under out/binaries holds this 572-byte span.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentFanOfSixteenGouraudTrianglesFromOriginAtRadius448TintedByWorkByteATimesEightAtOriginAndOneAtRimBmagicMagic09503_801EFDDC(
    void) {
  u8 *work;
  s32 *radius;
  s32 *base;
  s32 *tint;
  u16 *next;
  SVECTOR *shape;
  POLY_G3 *prim;
  long depth;
  long flag;
  s32 graph;
  s32 phase;
  s32 i;
  s32 one;

  radius = &D_1F800000;
  *radius = 0x1C0;

  next = &D_1F800024;
  *next = (func_801782FC(0) * *radius) >> 12;
  SPAD_REF(u16, 0x26u) = (func_801783C8(0) * *radius) >> 12;

  i = 1;
  shape = (SVECTOR *)(next - 8);
  one = 1;
  base = radius;
  tint = base + 2;
  do {
    phase = i << 8;
    shape[0].vx = 0;
    shape[0].vy = 0;
    shape[0].vz = 0;
    shape[1].vz = 0;
    shape[1].vx = shape[2].vx;
    shape[1].vy = shape[2].vy;
    shape[2].vx = (func_801782FC(phase) * *base) >> 12;
    shape[2].vy = (func_801783C8(phase) * *base) >> 12;
    shape[2].vz = 0;
    if (GetGraphType() == one || GetGraphType() == 2) {
      graph = 0xA5;
    } else {
      graph = 0x35;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
    func_80155A08(*(s32 *)(g_battle_work + 0x34),
                  *(s32 *)(g_battle_work + 0x38), 2, 0xC);
    prim = (POLY_G3 *)g_PrimCursor;
    i++;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);
    work = g_battle_work;
    *tint = work[0xA] << 3;
    prim->r0 = *tint;
    prim->g0 = *tint;
    prim->b0 = *tint;
    prim->r1 = one;
    prim->g1 = one;
    prim->b1 = one;
    prim->r2 = one;
    prim->g2 = one;
    prim->b2 = one;
    RotTransPers3(shape, shape + 1, shape + 2, (long *)&prim->x0,
                  (long *)&prim->x1, (long *)&prim->x2, &depth, &flag);
    func_80155A08(*(s32 *)(g_battle_work + 0x34),
                  *(s32 *)(g_battle_work + 0x38), 2, 0x1C);
  } while (i < 0x11);
}
