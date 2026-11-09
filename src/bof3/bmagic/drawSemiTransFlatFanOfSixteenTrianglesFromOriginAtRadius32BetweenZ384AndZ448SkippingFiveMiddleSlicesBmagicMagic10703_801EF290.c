#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work. This body re-forms the cell at each of its three read
 * sites, because a call and a run of primitive stores sit between them: the
 * opening `lui 0x1F80` plus `lw 0x44` pair precedes the frame setup, a second
 * pair follows RotTransPers3 for the tint test, and a third pair follows the
 * tint stores for the closing packet link. Only the first site feeds a local,
 * the masked bit the SemiTrans flag and the texture-page selection reuse across
 * every call. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius word at 0x1F800000, the scale both rotator samples of every
 * triangle are multiplied by. The original stores it once before the loop with
 * `sw` and reloads it with `lw` after each sampler call, the word width the
 * signed `mult` of the products shows, and it keeps the address in $s4 for the
 * whole body, which is why the loop reaches the tint cell below as $s4 plus
 * 0xC. */
extern s32 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad colour word at 0x1F80000C, the green drive this body publishes in
 * the non-flagged tint path and immediately reads back for the primitive green
 * byte, the alias the primitive stores force. The read is the absolute
 * `lui $v1` plus `lw 0xC` form, while the two publishes are `sw` at $s4 plus
 * 0xC. */
extern u32 D_1F80000C; /* @source 0x1F80000C @kind data */

/* Scratchpad next-sample cell at 0x1F800024, the x halfword this body seeds
 * before the loop and carries forward each pass. The original forms this
 * address into $s1 with its own `lui 0x1F80` plus `addiu 0x24` pair before the
 * first sampler call and later derives the vertex triplet base by subtracting
 * 0x10 from that register, so the three vertices are reached through this cell
 * rather than through a symbol at their first halfword. Its y neighbour at
 * 0x1F800026 and the two z halfwords at 0x1F800028 and 0x1F800018 are
 * published through literal constants instead, the absolute `lui $at` plus
 * `sh` form the compiler emits for a direct fixed-address store.
 */
extern u16 D_1F800024; /* @source 0x1F800024 @kind data */

/* Shared rotator pair at 0x801782FC and 0x801783C8: the samples of a 12-bit
 * turn in the fixed-point scale the `>> 12` products below divide out. This
 * body takes the 0x801782FC sample for the x halfword and the 0x801783C8
 * sample for the y halfword of both the pre-loop seed and every pass. Both the
 * pre-loop call and the per-pass calls pass the phase as their only argument. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared primitive linker at 0x80155A08: it resolves the (x, y) screen cell
 * through the shared cell-size pair, links the primitive at g_PrimCursor with
 * the given kind and advances the cursor by the given size. This body links
 * the 0xC-byte packet at the work object projection words 0x34 and 0x38
 * before the fan and the 0x14-byte flat triangle after each pass, both with
 * kind 2. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared libgpu POLY_F3 setter, the four-word flat-triangle packet the `addiu
 * $a1` of the packet link sizes at 0x14. It sits between the cursor read and
 * SetSemiTrans, the position this target's exact draw-group siblings give
 * their own primitive seed helper, and the payload names it only from this
 * span, so the @source tag carries the address. */
extern void func_8017A954(void *arg0); /* @source 0x8017A954 */

/* Shared graphic-type query feeding the draw-mode texture-page argument. */
s32 GetGraphType(void);

/* Shape evidence, not clone proof: the payload holds no byte-identical copy of
 * this 692-byte span (the 0x1F800000 and 0x1F800024 cells and the 0x180/0x1C0
 * depth pair appear only here), so the clone lever is ineligible and the body
 * is a clean-C seed. The statement order and the scratchpad access forms follow
 * the exact bmagic sibling
 * drawTranslucentFanOfSixteenGouraudTrianglesFromOriginAtRadius100TintedBy
 * WorkByte9WeightedEightTwoOneBmagicMagic00603_801F0790, which shares this
 * body's opening draw-mode selection, its pre-loop sampler seed through the
 * 0x1F800024 cell, its vertex-triplet carry into the second vertex and its
 * sixteen-step phase walk, and the exact same-target sibling
 * drawTranslucentGouraudTriangleFromProjectedOriginAlongZAtTwoDepthSteps
 * FadingFromWorkByteATimesSixToBlackBmagicMagic10703_801EF0E4, which shares the
 * literal-address z stores and the g_battle_work cell handling. */
/* @source 0x801EF290
 * @behavior Semi-transparent flat-triangle fan emitter of this overlay draw
 *           group with no caller in the payload: no `jal`, no jump and no
 *           pointer word of any extracted binary names 0x801EF290, and the body
 *           reads no incoming register, so the overlay loader entry is its only
 *           route and it takes no arguments and returns nothing.
 *           It first reads the scratchpad work-object pointer cell at
 *           0x1F800044 once and keeps the low bit of the object unsigned byte
 *           0x01 as a local for the whole body, picks the draw-mode
 *           texture-page argument from it - 0x25 or 0x15 with that bit shifted
 *           in at 7 or 5 places, the switch decided by GetGraphType returning 1
 *           or 2, called once per test with no cached answer - arms SetDrawMode
 *           on the primitive cursor g_PrimCursor with dfe 0, dtd 1 and a null
 *           RECT, and links the 0xC-byte packet at the work object projection
 *           words 0x34 and 0x38 with kind 2.
 *           It then publishes the numbers the fan runs on: 0x20 into the
 *           scratchpad radius word 0x1F800000, and a phase-zero seed sample -
 *           func_801782FC(0) times the radius shifted right twelve into the x
 *           halfword 0x1F800024, func_801783C8(0) times the same radius shifted
 *           right twelve into its y neighbour 0x1F800026, and 0x1C0 into the
 *           third halfword 0x1F800028. The radius is reloaded from the cell
 *           after each sampler call, so every product is the same published
 *           scale.
 *           The loop runs sixteen passes with the phase advancing by 0x100 each
 *           time, one sixteenth of the 0x1000 turn, so the sixteen wedges span
 *           the whole turn at one radius. Each pass carries the previous pass
 *           sample into the second vertex of the scratchpad triplet at
 *           0x1F800014, sets the first vertex to the origin x and y pair with
 *           z 0x180, and fills the third vertex with the samples of the current
 *           phase scaled by the radius and shifted right twelve at z 0x1C0, so
 *           every wedge is a flat triangle from the origin to the disc at one
 *           radius between the two depth planes.
 *           The pass then skips the primitive when its index minus eight is
 *           below five unsigned, leaving the five middle wedges unemitted.
 *           Otherwise it takes a fresh g_PrimCursor primitive, seeds it through
 *           func_8017A954, marks it semi-transparent with the work-object bit
 *           read at entry, projects the three vertices through RotTransPers3
 *           into the POLY_F3 vertex pairs at 0x08, 0x0C and 0x10 with two frame
 *           words as the depth and flag results, and writes one flat colour.
 *           The work-object bit is re-read from the cell after the projection:
 *           when it is set the colour is work-object unsigned byte 0x0A times
 *           six in red with 1 in green and blue, otherwise the red byte is
 *           0xC0 and the blue byte 0x10 while the green byte is the published
 *           0x1F80000C word - 0x91 minus sixteen times the pass index below
 *           index eight, and the running cell that starts at -0x5F and advances
 *           0x10 per pass from index eight on. func_80155A08 then links the
 *           0x14-byte flat triangle at the same record words with kind 2.
 *           The 0x48-byte frame keeps $ra, $s5, $s4, $s3, $s2, $s1 and $s0 plus
 *           the fifth SetDrawMode argument at 0x10, the four RotTransPers3 stack
 *           arguments at 0x10 to 0x1C and the sampler outputs at 0x20 and 0x24.
 *           The roles of the work-object byte 0x01 bit 0 and of byte 0x0A are
 *           not established beyond the tint and flag uses above, so they stay
 *           offset-anchored, and the scratchpad cells are named by address.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawSemiTransFlatFanOfSixteenTrianglesFromOriginAtRadius32BetweenZ384AndZ448SkippingFiveMiddleSlicesBmagicMagic10703_801EF290(
    void) {
  POLY_F3 *prim;
  SVECTOR *shape;
  u16 *next;
  s32 *cells;
  /* Same cell as cells, taken as its own local: the original keeps the
   * pre-loop base in $s0 and the loop base in $s4 and copies one into the
   * other before the loop, which only a second live value reproduces. */
  s32 *radius;
  long depth;
  long flag;
  s32 graph;
  s32 semi;
  s32 phase;
  s32 shade;
  s32 i;

  semi = g_battle_work[1] & 1;
  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = (semi << 7) | 0x25;
  } else {
    graph = (semi << 5) | 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38), 2,
                0xC);

  cells = &D_1F800000;
  *cells = 0x20;
  next = &D_1F800024;
  *next = (func_801782FC(0) * *cells) >> 12;
  *(u16 *)0x1F800026 = (func_801783C8(0) * *cells) >> 12;
  *(u16 *)0x1F800028 = 0x1C0;
  i = 1;
  shape = (SVECTOR *)(next - 8);
  radius = cells;
  shade = -0x5F;
  do {
    phase = i << 8;
    shape[0].vx = 0;
    shape[0].vy = 0;
    shape[0].vz = 0x180;
    shape[1].vx = shape[2].vx;
    shape[1].vy = shape[2].vy;
    shape[1].vz = shape[2].vz;
    shape[2].vx = (func_801782FC(phase) * *radius) >> 12;
    shape[2].vy = (func_801783C8(phase) * *radius) >> 12;
    shape[2].vz = 0x1C0;

    if ((u32)(i - 8) >= 5) {
      prim = (POLY_F3 *)g_PrimCursor;
      func_8017A954(prim);
      SetSemiTrans(prim, semi);

      RotTransPers3(shape, shape + 1, shape + 2, (long *)&prim->x0,
                    (long *)&prim->x1, (long *)&prim->x2, &depth, &flag);

      if (g_battle_work[1] & 1) {
        prim->r0 = g_battle_work[0xA] * 6;
        prim->g0 = 1;
        prim->b0 = 1;
      } else {
        if (i < 8) {
          *(radius + 3) = 0x91 - (i << 4);
        } else {
          *(radius + 3) = shade;
        }
        prim->r0 = 0xC0;
        prim->g0 = D_1F80000C;
        prim->b0 = 0x10;
      }

      func_80155A08(*(s32 *)(g_battle_work + 0x34),
                    *(s32 *)(g_battle_work + 0x38), 2, 0x14);
    }
    i++;
    shade += 0x10;
  } while (i < 0x11);
}
