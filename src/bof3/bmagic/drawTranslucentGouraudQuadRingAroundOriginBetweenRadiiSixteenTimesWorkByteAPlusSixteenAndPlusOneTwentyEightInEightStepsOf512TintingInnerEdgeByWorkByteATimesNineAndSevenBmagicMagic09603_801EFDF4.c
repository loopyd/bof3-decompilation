#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's own
 * map row g_battle_work. Every site that reads work bytes or the two work words
 * 0x34 and 0x38 through it materializes its own `lui`/`lw` pair, so the body
 * never keeps the cell in a register across a call. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad halfword block at 0x1F800000. The body addresses it two ways in
 * the original: the inner radius cell 0x1F800000 through the block base and the
 * two tint cells 0x1F80000C and 0x1F80000E through one base register with the
 * 0xC and 0xE displacements, so the block is bound as a halfword array whose
 * elements 6 and 7 are those tint cells. */
extern u16 D_1F800000; /* @source 0x1F800000 @kind data */

/* Scratchpad outer-radius cell at 0x1F800002. It is its own symbol, so every
 * load of it materializes the fixed address with its own `lui`/`lh` pair
 * instead of joining the block base, and the loop reaches both radii through
 * one base pointer at 0x1F800002 with the 0 and -2 displacements. */
extern u16 D_1F800002; /* @source 0x1F800002 @kind data */

/* Scratchpad phase point at 0x1F80001C. The original materializes this address
 * once into a callee-saved register, stores the outer sine and cosine samples
 * through it and derives the four-vector shape handed to RotTransPers4 from it
 * by four halfwords, so the shape base is its -4 displacement. */
extern u16 D_1F80001C; /* @source 0x1F80001C @kind data */

/* Shared object-table link helper, called with the scratchpad work words 0x34
 * and 0x38, kind 2 and the packet size. No in-image symbol names 0x80155A08. */
extern void func_80155A08(s32 x, s32 y, s32 kind, s32 size); /* @source 0x80155A08 */

/* Shared phase samplers at 0x801782FC and 0x801783C8: the sine and cosine
 * samples of a 12-bit phase in the fixed-point scale the `>> 12` products below
 * undo. Both are called with 0 for the phase-zero points and with the step
 * phase afterwards. */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */

/* Shared graphic-type query. The SDK map names 0x8017B2B4. */
extern s32 GetGraphType(void); /* @source 0x8017B2B4 */

/* Bytes 0x801EFDF4..0x801F0163 occur in no other payload under out/binaries, so
 * this span has no byte-identical sibling and the clone lever was declined. The
 * statement order, local shapes and primitive field forms follow two siblings
 * that recorded exact measurements when this source was written: the same
 * target's drawTranslucentFanOfEightGouraudTrianglesFromOriginAtRadiusSixteen
 * TimesWorkByteAPlusSixteenTintedByWorkByteATimesSevenEightAtOriginAndNineAt
 * RimBmagicMagic09603_801EFB3C, which seeds the same scratchpad block, the same
 * three cleared z halfwords and the same shared-helper call order, and
 * drawEightTranslucentGouraudRingQuadsAtWorkHalfwords2EAnd30BmagicMagic13003_
 * 801EF730, which walks eight quads in 0x200 steps over two scratchpad radii
 * addressed as one base pointer plus one and its -1 element. Every literal,
 * cell, tint offset, packet size and vertex order here comes from this span's
 * own bytes.
 * @source 0x801EFDF4
 * @behavior Draws a translucent Gouraud ring of eight quads around the origin.
 *           It selects draw-mode texture page 0xA5 when the shared graphic type
 *           is 1 or 2 and 0x35 otherwise, installs it at g_PrimCursor with dfe
 *           0 and dtd 1, and links a 0xC-byte draw-mode packet through the
 *           helper at 0x80155A08 with work words 0x34 and 0x38 and kind 2. Each
 *           of the two graph-type tests keeps its own call.
 *           It then seeds four scratchpad halfwords from unsigned work byte
 *           0xA: the inner signed radius at 0x1F800000 is sixteen times that
 *           byte plus 16 and the outer signed radius at 0x1F800002 is sixteen
 *           times that byte plus 128, while 0x1F80000C and 0x1F80000E hold nine
 *           and seven times that byte. The phase-zero outer point at 0x1F80001C
 *           and 0x1F80001E is the sine and the cosine sample of phase zero
 *           scaled by the outer radius and shifted right 12, the phase-zero
 *           inner point at 0x1F80002C and 0x1F80002E is the same pair scaled by
 *           the inner radius, and the three vertex z halfwords at 0x1F800018,
 *           0x1F800020 and 0x1F800028 are cleared once, before the loop.
 *           It emits eight quads, one per 0x200 step of the 0x1000 circle. A
 *           quad is a semi-transparent Gouraud POLY_G4 seeded by SetPolyG4 whose
 *           four vertices are the previous outer point, the current outer point,
 *           the previous inner point and the current inner point: each step
 *           carries the two previous points forward, recomputes the outer point
 *           from the outer radius and the inner point from the inner radius at
 *           the step phase, and projects all four with RotTransPers4 onto the
 *           cursor primitive. Every quad closes with the same 0x24-byte packet
 *           link as the draw mode.
 *           The two outer vertices take the colour 1, 1, 1 and the two inner
 *           vertices take work byte 0xA times nine, work byte 0xA times seven
 *           and work byte 0xA times nine through the tint cells 0x1F80000C and
 *           0x1F80000E, each read separately because the primitive stores may
 *           alias the cells.
 *           The 0x50-byte frame keeps ra at 0x4C, s0 to s6 at 0x30 to 0x48, the
 *           four outgoing RotTransPers4 stack arguments at 0x10 to 0x1C and the
 *           projected depth and flag the call publishes at 0x28 and 0x2C. It
 *           takes no arguments, returns nothing and writes no state outside the
 *           scratchpad cells, the cursor and the primitives it emits.
 * @status exact
 * @match 100.00
 * @residual none
 */

void drawTranslucentGouraudQuadRingAroundOriginBetweenRadiiSixteenTimesWorkByteAPlusSixteenAndPlusOneTwentyEightInEightStepsOf512TintingInnerEdgeByWorkByteATimesNineAndSevenBmagicMagic09603_801EFDF4(
    void) {
  u16 *cells;
  u16 *next;
  s16 *outer;
  u16 *tint;
  POLY_G4 *primitive;
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
  D_1F800002 = (g_battle_work[0xA] << 4) + 0x80;
  *(u16 *)0x1F80000C = g_battle_work[0xA] * 9;
  *(u16 *)0x1F80000E = g_battle_work[0xA] * 7;

  next = &D_1F80001C;
  *next = (func_801782FC(0) * (s16)D_1F800002) >> 12;
  *(u16 *)0x1F80001E = (func_801783C8(0) * (s16)D_1F800002) >> 12;
  *(u16 *)0x1F80002C = (func_801782FC(0) * *(s16 *)cells) >> 12;
  *(u16 *)0x1F80002E = (func_801783C8(0) * *(s16 *)cells) >> 12;

  i = 1;
  shape = (SVECTOR *)(next - 4);
  outer = (s16 *)(cells + 1);
  tint = cells;
  *(u16 *)0x1F800028 = 0;
  *(u16 *)0x1F800020 = 0;
  *(u16 *)0x1F800018 = 0;
  do {
    primitive = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(primitive);
    SetSemiTrans(primitive, 1);

    phase = i << 9;
    shape[0].vx = shape[1].vx;
    shape[0].vy = shape[1].vy;
    shape[1].vx = (func_801782FC(phase) * outer[0]) >> 12;
    shape[1].vy = (func_801783C8(phase) * outer[0]) >> 12;
    shape[2].vx = shape[3].vx;
    shape[2].vy = shape[3].vy;
    shape[3].vx = (func_801782FC(phase) * outer[-1]) >> 12;
    shape[3].vy = (func_801783C8(phase) * outer[-1]) >> 12;

    RotTransPers4(shape, shape + 1, shape + 2, shape + 3, (long *)&primitive->x0,
                  (long *)&primitive->x1, (long *)&primitive->x2,
                  (long *)&primitive->x3, &depth, &flag);

    primitive->r0 = 1;
    primitive->g0 = 1;
    primitive->b0 = 1;
    primitive->r1 = 1;
    primitive->g1 = 1;
    primitive->b1 = 1;
    primitive->r2 = tint[6];
    primitive->g2 = tint[7];
    primitive->b2 = tint[6];
    primitive->r3 = tint[6];
    primitive->g3 = tint[7];
    primitive->b3 = tint[6];

    i++;
    func_80155A08(FIELD_REF(s32, g_battle_work, 0x34),
                  FIELD_REF(s32, g_battle_work, 0x38), 2, 0x24);
  } while (i < 9);
}
