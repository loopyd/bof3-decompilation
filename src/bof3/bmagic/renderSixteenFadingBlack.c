#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work. This body re-forms the cell at each of its four read sites,
 * because a call sits between them: the byte-2 pulse test precedes the draw-mode
 * pair, each radius arm reads byte 9 after its own sampler call, and the two
 * tint bytes come from one pointer load that both stores reuse. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad radius halfword at 0x1F800000, the signed scale both rotator
 * samples of every triangle are multiplied by. The original publishes it with
 * the absolute `lui $at` plus `sh` form and reloads it with `lh` off the register
 * that this body derives by subtracting 6 from its 0x1F800006 tint pointer. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad tint halfword at 0x1F800006, the green and blue drive of the
 * primitive colour triplet, published once before the fan through the pointer
 * this body keeps in a saved register. Its neighbour at 0x1F800008, the red
 * drive, is published through a literal-address constant instead, the absolute
 * `lui $at` plus `sh` form the compiler emits for a direct fixed-address store. */
extern u16 D_1F800006; /* @source 0x1F800006 @kind data */

/* Scratchpad x halfword of the carried rim point at 0x1F800024, seeded before
 * the loop and rewritten by every pass; its y neighbour at 0x1F800026 is
 * published through a literal constant and the three vector z halfwords at
 * 0x1F800018, 0x1F800020 and 0x1F800028 are cleared the same way. */
extern u16 D_1F800024; /* @source 0x1F800024 @kind data */

/* Shared primitive seed helper for the 0x1C-byte POLY_G3 the fan emits: it sits
 * between the cursor read and SetSemiTrans, the position the exact bmagic draw
 * siblings give their own seed helper. */
extern void func_8017A97C(void *primitive); /* @source 0x8017A97C @kind function */

/* Shared phase samplers at 0x801782FC and 0x801783C8: the sine and cosine of a
 * 12-bit phase in the fixed-point scale the `>> 12` products below divide out.
 * This body takes the 0x801782FC sample for the x halfword and the 0x801783C8
 * sample for the y halfword of both the pre-loop seed and every pass. */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC @kind function */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 @kind function */

extern s32 GetGraphType(void);

/* @source 0x801EFC74
 * @behavior Semi-transparent Gouraud fan emitter of this overlay draw group
 *           reached from the work record's byte-2 handler arm, which calls it
 *           between the transform installer at 0x801EF410 and PopMatrix with no
 *           arguments. It selects the draw-mode texture page from the graph
 *           type, 0x125 for types 1 and 2 and 0x55 otherwise, publishes the mode
 *           on the primitive cursor and appends its 0xC-byte packet to ordering
 *           bucket 5.
 *           It then sets the scratchpad radius halfword at 0x1F800000 from the
 *           work record: when byte 2 reads at least 2 the radius is
 *           (byte 9 * 4 + 0x210) scaled by the 0x801783C8 sample of half turn
 *           over 0x1000, and while byte 2 reads below 2 it is byte 9 * 0x30
 *           scaled by that same sample, the arm the branch takes for the first
 *           two states. It publishes byte A * 8 at 0x1F800006 and byte A * 6 at
 *           0x1F800008 as the tint drives, seeds the carried rim point at
 *           0x1F800024/0x1F800026 from the sine and cosine of phase 0, and
 *           clears the three vector z halfwords.
 *           Sixteen passes then walk the phase in steps of 0x100. Each reads a
 *           fresh cursor, seeds and marks it semi-transparent, carries the
 *           previous rim point into the second vertex, scales the sine and
 *           cosine of the phase by the signed radius for the third, projects
 *           the three scratchpad vectors with RotTransPers3 and appends the
 *           0x1C-byte packet to bucket 5. The first vertex takes the tint drives
 *           (red 0x1F800008, green and blue 0x1F800006) and both rim vertices are
 *           black, so the fan fades from the tint at the origin to black.
 *           It takes no arguments, returns nothing and its 0x40-byte frame saves
 *           $ra and $s0-$s4, keeping only the RotTransPers3 depth and flag words
 *           at 0x20 and 0x24.
 *
 *           Statement order, scratchpad access forms and declaration style follow
 *           the exact bmagic siblings
 *           drawTranslucentEightTriangleFanWithPulsingRadiusAndWorkByteATint at
 *           0x801EF07C, which shares this body's draw-mode selection, its
 *           pulsing radius, its byte-A tint pair and its cursor-first loop, and
 *           drawTranslucentFanOfSixteenGouraudTrianglesFromOriginAtRadius240 at
 *           0x801EF68C, which shares the sixteen-step phase walk, the carried rim
 *           point and the projection call. The 0x2B4-byte body is not
 *           byte-identical to any span of the 398 payloads under out/binaries,
 *           so the clone lever is ineligible and this is a clean-C seed.
 *           Representation notes: the byte-2 arm that scales the radius by 0x30
 *           needs its constant attached to the sampler product. Spelled
 *           as `func_801783C8(0x800) * 48`, `* 0x30`, `* 3 * 16`, `* 16 * 3` or
 *           through a `scale` local, the compiler moved the constant onto the
 *           record load and scored 165 of 175 instructions at 700 bytes; the
 *           explicit `(func_801783C8(0x800) * 3) << 4` left the factor on the
 *           sample and closed the diff. The three vector z halfwords are cleared
 *           before the loop through literal scratchpad addresses: written as
 *           `shape[n].vz` inside the loop they stay loop-relative and cost three
 *           instructions.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderSixteenFadingBlack(
    void) {
  u16 *tint;
  u16 *next;
  u16 *cells;
  POLY_G3 *primitive;
  SVECTOR *shape;
  long depth;
  long flag;
  s32 tpage;
  s32 phase;
  s32 i;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0x125;
  } else {
    tpage = 0x55;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  GpuAppendPrim(5, 0xC);

  if (g_battle_work[2] >= 2) {
    D_1F800000 = ((g_battle_work[9] * 4 + 0x210) * func_801783C8(0x800)) >> 12;
  } else {
    D_1F800000 = (((func_801783C8(0x800) * 3) << 4) * g_battle_work[9]) >> 12;
  }
  tint = &D_1F800006;
  *tint = g_battle_work[0xA] * 8;
  SPAD_REF(u16, 8) = g_battle_work[0xA] * 6;

  next = &D_1F800024;
  *next = (func_801782FC(0) * SPAD_REF(s16, 0x00u)) >> 12;
  SPAD_REF(u16, 0x26u) = (func_801783C8(0) * *(s16 *)&D_1F800000) >> 12;

  SPAD_REF(u16, 0x28u) = 0;
  SPAD_REF(u16, 0x20u) = 0;
  SPAD_REF(u16, 0x18u) = 0;
  i = 1;
  shape = (SVECTOR *)(next - 8);
  cells = tint - 3;
  do {
    primitive = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(primitive);
    SetSemiTrans(primitive, 1);

    phase = i << 8;
    shape[0].vx = 0;
    shape[0].vy = 0;
    shape[1].vx = shape[2].vx;
    shape[1].vy = shape[2].vy;
    shape[2].vx = (func_801782FC(phase) * *(s16 *)cells) >> 12;
    shape[2].vy = (func_801783C8(phase) * *(s16 *)cells) >> 12;
    primitive->r0 = cells[4];
    primitive->g0 = cells[3];
    primitive->b0 = cells[3];
    primitive->r1 = 0;
    primitive->g1 = 0;
    primitive->b1 = 0;
    primitive->r2 = 0;
    primitive->g2 = 0;
    primitive->b2 = 0;
    RotTransPers3(shape, shape + 1, shape + 2, (long *)&primitive->x0,
                  (long *)&primitive->x1, (long *)&primitive->x2, &depth,
                  &flag);

    GpuAppendPrim(5, 0x1C);
    i++;
  } while (i < 0x11);
}
