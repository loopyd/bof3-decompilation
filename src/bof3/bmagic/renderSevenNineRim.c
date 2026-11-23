#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's own
 * map row g_battle_work. Every site that reads work bytes or the two work words
 * 0x34 and 0x38 through it materializes its own `lui`/`lw` pair, so the body
 * never keeps the cell in a register across a call. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad halfword block at 0x1F800000. This body addresses it two ways in
 * the original: the radius cell 0x1F800000 and the three tint cells 0x1F80000A,
 * 0x1F80000C and 0x1F80000E, which the loop re-reads through one base register
 * with the 0xA, 0xC and 0xE displacements, so the block is bound as a halfword
 * array and the tint cells are its 5, 6 and 7 elements. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad phase-zero fan point at 0x1F800024. The original materializes this
 * address once into a callee-saved register, stores the sine sample through it
 * and derives the three-vector shape handed to RotTransPers3 from it by eight
 * halfwords, so the shape base is its 0x10 displacement. */
extern u16 D_1F800024; /* @source 0x1F800024 @kind data */

/* Shared primitive seed helper, called on the fresh cursor primitive of every
 * step before this body writes its fields. No in-image symbol names 0x8017A97C,
 * so it keeps its address name. */
extern void func_8017A97C(void *primitive); /* @source 0x8017A97C */

/* Shared object-table link helper, called with the scratchpad work words 0x34
 * and 0x38, kind 2 and the packet size. No in-image symbol names 0x80155A08. */
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size); /* @source 0x80155A08 */

/* Shared phase samplers at 0x801782FC and 0x801783C8: the sine and cosine
 * samples of a 12-bit phase in the fixed-point scale the `>> 12` products below
 * undo. Both are called with 0 for the phase-zero fan point and with the step
 * phase afterwards. */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */

/* Shared graphic-type query. The SDK map names 0x8017B2B4. */
extern s32 GetGraphType(void); /* @source 0x8017B2B4 */

/* Bytes 0x801EFB3C..0x801EFDF3 occur in no other payload under out/binaries, so
 * this span has no byte-identical sibling and the clone lever was declined. The
 * statement order, local shapes and primitive field forms follow the exact
 * sibling
 * drawTranslucentFanOfEightGouraudTrianglesFromOriginAtRadiusTenTimesWorkByte9PlusSixTintedByWorkByteATimesSixBmagicMagic04603_801EFC48,
 * which recorded an exact measurement when this source was written and which
 * seeds the same scratchpad fan with one pointer to the halfword block, one to
 * the phase-zero point, a shape pointer derived from that point by eight
 * halfwords and a separate tint pointer copied from the first. Every literal,
 * cell, tint offset and packet size here comes from this span's own bytes.
 * @source 0x801EFB3C
 * @behavior Draws a translucent Gouraud fan of eight triangles around the
 *           origin. It selects draw-mode texture page 0xA5 when the shared
 *           graphic type is 1 or 2 and 0x35 otherwise, installs it at
 *           g_PrimCursor with dfe 0 and dtd 1, and links a 0xC-byte draw-mode
 *           packet through the helper at 0x80155A08 with work words 0x34 and
 *           0x38 and kind 2. Each of the two graph-type tests keeps its own
 *           call.
 *           It then seeds four scratchpad halfwords from unsigned work byte
 *           0xA: the signed radius at 0x1F800000 is sixteen times that byte plus
 *           16, and 0x1F80000A, 0x1F80000C and 0x1F80000E hold eight, nine and
 *           seven times that byte. The phase-zero fan point at 0x1F800024 and
 *           0x1F800026 is the sine and the cosine sample of phase zero scaled by
 *           the signed radius and shifted right 12, and the three vertex z
 *           halfwords at 0x1F800018, 0x1F800020 and 0x1F800028 are cleared once,
 *           before the loop.
 *           It emits eight triangles, one per 0x200 step of the 0x1000 circle.
 *           A triangle is a semi-transparent Gouraud POLY_G3 seeded by the
 *           shared helper at 0x8017A97C whose vertices are the origin, the
 *           previous ring point and the current ring point; the origin keeps the
 *           two z-cleared halfwords, the current point is the sampler pair of the
 *           step phase scaled by the radius and shifted right 12, and it becomes
 *           the next step's previous point through the two halfwords read back
 *           before the point is overwritten. Every triangle closes with the same
 *           0x1C-byte packet link as the draw mode.
 *           Every vertex of a triangle takes its colour from work byte 0xA
 *           through the three tint halfwords of the block: the origin vertex is
 *           elements 7, 7 and 5 and both ring vertices are elements 6, 7 and 6,
 *           each read separately because the primitive stores may alias the
 *           cells.
 *           The 0x40-byte frame keeps ra at 0x3C, s0 to s4 at 0x28 to 0x38, the
 *           four outgoing RotTransPers3 stack arguments at 0x10 to 0x1C and the
 *           projected depth and flag the call publishes at 0x20 and 0x24. It
 *           takes no arguments, returns nothing and writes no state outside the
 *           scratchpad cells, the cursor and the primitives it emits.
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderSevenNineRim(
    void) {
  u16 *cells;
  u16 *next;
  u16 *tint;
  POLY_G3 *primitive;
  SVECTOR *shape;
  long depth;
  long flag;
  s32 tpage;
  s32 phase;
  s32 i;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  func_80155A08(FIELD_REF(s32, g_battle_work, 0x34),
                FIELD_REF(s32, g_battle_work, 0x38), 2, 0xC);

  cells = &D_1F800000;
  *cells = (g_battle_work[0xA] << 4) + 0x10;
  *(u16 *)0x1F80000A = g_battle_work[0xA] << 3;
  *(u16 *)0x1F80000C = g_battle_work[0xA] * 9;
  *(u16 *)0x1F80000E = g_battle_work[0xA] * 7;
  next = &D_1F800024;
  *next = (func_801782FC(0) * *(s16 *)cells) >> 12;
  *(u16 *)0x1F800026 = (func_801783C8(0) * *(s16 *)cells) >> 12;

  i = 1;
  shape = (SVECTOR *)(next - 8);
  tint = cells;
  *(u16 *)0x1F800028 = 0;
  *(u16 *)0x1F800020 = 0;
  *(u16 *)0x1F800018 = 0;
  do {
    primitive = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(primitive);
    SetSemiTrans(primitive, 1);

    shape[0].vx = 0;
    shape[0].vy = 0;
    shape[1].vx = shape[2].vx;
    shape[1].vy = shape[2].vy;

    phase = i << 9;
    shape[2].vx = (func_801782FC(phase) * *(s16 *)tint) >> 12;
    shape[2].vy = (func_801783C8(phase) * *(s16 *)tint) >> 12;

    RotTransPers3(shape, shape + 1, shape + 2, (long *)&primitive->x0,
                  (long *)&primitive->x1, (long *)&primitive->x2, &depth,
                  &flag);

    i++;
    primitive->r0 = tint[7];
    primitive->g0 = tint[7];
    primitive->b0 = tint[5];
    primitive->r1 = tint[6];
    primitive->g1 = tint[7];
    primitive->b1 = tint[6];
    primitive->r2 = tint[6];
    primitive->g2 = tint[7];
    primitive->b2 = tint[6];

    func_80155A08(FIELD_REF(s32, g_battle_work, 0x34),
                  FIELD_REF(s32, g_battle_work, 0x38), 2, 0x1C);
  } while (i < 9);
}
