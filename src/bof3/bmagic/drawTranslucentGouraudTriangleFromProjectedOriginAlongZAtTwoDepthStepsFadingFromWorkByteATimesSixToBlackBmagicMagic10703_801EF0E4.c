#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map. This body dereferences it three times and the original gives each of the
 * three sites the same `lui 0x1F80` plus `lw 0x44` pair, because a call sits
 * between every pair of them: the first func_80155A08 call, RotTransPers3, and
 * then the second func_80155A08 call. Both func_80155A08 sites pass two
 * quantities read from one cell load, so those arguments dereference the cell
 * twice inside one expression and the compiler merges the two reads into the
 * single `lui`/`lw` pair the original shows, landing in $v0. The body never
 * keeps one base across the sites, so the cell stays unqualified. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad vertex triplet at 0x1F800014, 0x1F80001C and 0x1F800024. Its
 * address is formed once as one `lui $a0` plus `addiu $a0` pair and kept live,
 * because it is both the first RotTransPers3 source vector and the base the two
 * later vertex x fields are addressed from at offsets 0x8 and 0x10. The three x
 * stores reach it as base plus a constant offset - the middle one from the same
 * pointer and the far one from the second vertex pointer - which is what keeps
 * all three in the one-instruction `sh $zero, offset($a0)` form; spelling either
 * later one as an absolute address instead makes the compiler fold it into a
 * separate `lui $at` pair and shifts the whole store block out of the
 * original's order. The remaining six halfwords of the triplet are published
 * through literal constants instead, the absolute `lui $at` plus `sh` form the
 * compiler emits for a direct fixed-address store, and they interleave with the
 * three x stores in the original's order only in this spelling. */
extern u16 D_1F800014; /* @source 0x1F800014 @kind data */

/* Scratchpad word cell at 0x1F80000C, the colour word this body publishes and
 * then reads back six times. The address is formed once as `lui $a0` plus
 * `addiu $a0` and stays live in $a0 across the six byte stores, since the stores
 * through the primitive pointer may alias the cell and force a fresh `lw 0($a0)`
 * before each of them; the store itself reuses the register that holds the
 * product. A per-read absolute address would emit six extra address pairs the
 * original does not have, so the cell is a live base here rather than a folded
 * constant. */
extern u32 D_1F80000C; /* @source 0x1F80000C @kind data */

/* Shared primitive linker: resolves the (x, y) screen cell through the shared
 * cell-size pair at 0x80159320/0x80159322, links the primitive at g_PrimCursor
 * with the given kind and advances the cursor by the given size class. This body
 * links a 0xC-byte packet at the work object projection words 0x34 and 0x38
 * before the triangle and the 0x20-byte triangle itself after it, both with
 * kind 2. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared primitive seed helper: it sits between the cursor read and
 * SetSemiTrans, the position the exact bmagic siblings give their own seed
 * helper. */
void func_8017AAC8(void *arg0);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* @source 0x801EF0E4
 * @behavior Translucent Gouraud triangle emitter of this overlay's draw group:
 *           no `jal`, no pointer word and no table entry in any extracted binary
 *           names 0x801EF0E4, and the body reads no incoming register, so the
 *           overlay loader entry is its only route and it takes no arguments.
 *           It first picks the draw-mode tpage argument, 0xA5 when GetGraphType
 *           returns 1 or 2 (two calls, the second only on the first answering
 *           something else) and 0x35 otherwise, seeds SetDrawMode at
 *           g_PrimCursor with dfe 0, dtd 1 and a null RECT, and links the
 *           0xC-byte packet at the work-record words 0x34 and 0x38 with kind 2.
 *           It then takes the cursor primitive, seeds it through func_8017AAC8
 *           and marks it semi-transparent, clears the scratchpad vertex triplet
 *           at 0x1F800014 - all three x fields through the live base and the six
 *           y and z halfwords through literal constants - and sets the two later
 *           vertices to z 0x170 and 0x180, so the triangle runs from the
 *           projected origin along the current transform's z axis. The same
 *           triplet goes to RotTransPers3 with the primitive vertex pairs at
 *           0x08, 0x10 and 0x18 as its three screen outputs and two frame words
 *           as the depth and flag results. The colour word is
 *           work byte 0x0A times six, published to the scratchpad word at
 *           0x1F80000C and read back as the low byte of the r, g and b of the
 *           first two vertices, while the third vertex takes 1 in all three
 *           channels, so the triangle grades from work byte 0x0A times six at
 *           the near end to one at the far end. Finally func_80155A08 links the
 *           0x20-byte triangle at the same record words with kind 2. Keeps a
 *           0x30-byte frame for $ra, $s0 and the four outgoing words of
 *           RotTransPers3 plus the depth and flag results, and writes nothing
 *           outside the scratchpad and the primitive it emits. No payload under
 *           out/binaries/emi holds these 428 bytes, so the clone lever is
 *           ineligible; the statement order and declaration style follow the
 *           exact family sibling drawFourPhasedTrianglesFromOrigin
 *           TintedByWorkRecordLifetimeBmagicMagic10603_801EF5F4 and the
 *           translucent quad sibling drawTranslucentTexturedQuadAtWork
 *           Halfwords2EAnd30ScaledByWorkByte9TintedByWorkByteATimesBytes5d5e5f
 *           BmagicMagic04403_801F06D8, which share this body's opening draw-mode
 *           selection, its func_80155A08 packet pair and its scratchpad
 *           triplet.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudTriangleFromProjectedOriginAlongZAtTwoDepthStepsFadingFromWorkByteATimesSixToBlackBmagicMagic10703_801EF0E4(
    void) {
  POLY_G3 *prim;
  SVECTOR *shape;
  SVECTOR *mid;
  SVECTOR *far;
  u32 *tint;
  s32 graph_value;
  long depth;
  long flag;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph_value = 0xA5;
  } else {
    graph_value = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph_value, 0);
  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38), 2,
                0xC);

  prim = (POLY_G3 *)g_PrimCursor;
  func_8017AAC8(prim);
  SetSemiTrans(prim, 1);

  shape = (SVECTOR *)&D_1F800014;
  mid = shape + 1;
  far = mid + 1;
  *(u16 *)0x1F800016 = 0;
  *(u16 *)shape = 0;
  *(u16 *)0x1F800018 = 0;
  *(u16 *)0x1F80001E = 0;
  *(u16 *)((u8 *)shape + 8) = 0;
  *(u16 *)0x1F800020 = 0x170;
  *(u16 *)0x1F800026 = 0;
  *(u16 *)((u8 *)mid + 8) = 0;
  *(u16 *)0x1F800028 = 0x180;

  RotTransPers3(shape, mid, far, (long *)&prim->x0, (long *)&prim->x1,
                (long *)&prim->x2, &depth, &flag);

  tint = &D_1F80000C;
  *tint = g_battle_work[0xA] * 6;
  prim->r0 = *tint;
  prim->g0 = *tint;
  prim->b0 = *tint;
  prim->r1 = *tint;
  prim->g1 = *tint;
  prim->b1 = *tint;
  prim->r2 = 1;
  prim->g2 = 1;
  prim->b2 = 1;

  func_80155A08(*(s32 *)(g_battle_work + 0x34), *(s32 *)(g_battle_work + 0x38), 2,
                0x20);
}
