#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared primitive seed helper at 0x8017A97C, called on the fresh cursor
 * primitive of every step before this body writes its fields. No symbol in the
 * main executable names that address, so it keeps its raw name. */
void func_8017A97C(void *arg0); /* @source 0x8017A97C */

/* Shared phase samplers at 0x801782FC and 0x801783C8: the sine and cosine
 * samples of a 12-bit phase in the fixed-point scale the `>> 12` products below
 * undo. Both are called with the arm vertex angle and then with the angle one
 * step further round the turn. */
s32 func_801782FC(s32 arg0); /* @source 0x801782FC */

s32 func_801783C8(s32 arg0); /* @source 0x801783C8 */

/* @source 0x801EF790
 * @behavior Draws one closed translucent red disc: sixteen gouraud triangles
 *   that share the centre point and cover a whole turn, one 0x100 step of the
 *   0x1000 circle each, so the last triangle closes the ring on the first.
 *   Every step reloads the cursor primitive from g_PrimCursor, seeds it with
 *   func_8017A97C and turns semi-transparency on, then writes the three
 *   vertices of a POLY_G3: vertex 0 is the centre pair, vertex 1 is the arc
 *   point at the step angle and vertex 2 the arc point one step further. An
 *   arc coordinate is the centre coordinate plus the signed radius times the
 *   sampler of that angle, shifted right twelve; the x coordinate takes
 *   func_801783C8 and the y coordinate func_801782FC, so the step angle is
 *   sampled twice per arc vertex.
 *   Only the red channel carries the shades: vertex 0 takes the fourth
 *   argument, vertices 1 and 2 take the fifth, and the green and blue byte of
 *   every vertex is cleared, leaving the two pad bytes of each colour word as
 *   the previous primitive left them. The 0x1C-byte POLY_G3 is then appended at
 *   ordering-table index 1.
 *   The counter is kept twice over: the current angle is an unsigned halfword
 *   truncated from the full-word phase before the first sampler pair, while the
 *   arc point is the phase one 0x100 step ahead, held as a full word and
 *   truncated only where the two samplers of the second pair read it and where
 *   the loop test compares it with 0x1000; the phase then takes that next value,
 *   so the last step runs the phase 0xF00 and the test that ends the loop reads
 *   0x1000. The frame is 0x38 bytes with $ra at 0x34 and $s0 to $s7 plus $fp
 *   saved above the sixteen-byte argument home area, which carries no outgoing
 *   arguments: the body makes no stack calls.
 *   It takes five arguments - two wide centre coordinates, the radius, and the
 *   centre and arc shades, the last of the two on the stack and read back with
 *   an unsigned byte load - and returns nothing. It writes no state outside
 *   the primitives it emits through the cursor.
 *   Its only in-image caller is the draw of 0x801EFA18 inside func_801EF8EC,
 *   which was still an asm row when this body was written: it projects a slot of
 *   the table at 0x801EFAB4, hands over the projected pair and the slot's
 *   radius halfword with 0x80 as the centre shade and 0 for the arc, so the
 *   disc fades from a half-bright centre to an unlit rim.
 *   Shape evidence: no payload under out/binaries holds a twin of bytes
 *   0x801EF790..0x801EF8EB - the closest window anywhere differs in 37 words
 *   and this overlay's own copy is the only exact one - so the clone lever was
 *   declined and the body was seeded from the exact sibling
 *   drawTranslucentRadialFanOfThirtyTwoGouraudTrianglesTintedByCenterAndArcShade
 *   at 0x801F4138 of bmagic world04 area172, whose loop repeats this statement
 *   order, vertex field order and 0x1C-byte append with a 0x80 step, a byte
 *   counter and shades clamped to 0..0xFF into all three channels. Its
 *   statement order, operand forms and declaration style were transcribed while
 *   the step, the 0x1000 bound, the five-argument contract, the red-only stores
 *   and every sampler call here come from this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentRedRadialFanOfSixteenGouraudTrianglesTintedByCenterAndArcShadeBmagicMagic15003_801EF790(
    s32 centre_x, s32 centre_y, s16 radius, u8 centre_shade, u8 arc_shade) {
  POLY_G3 *prim;
  u16 angle;
  s32 next;
  s32 phase;

  phase = 0;
  do {
    prim = (POLY_G3 *)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);
    angle = phase;
    next = phase + 0x100;
    prim->x0 = centre_x;
    prim->y0 = centre_y;
    prim->x1 = centre_x + ((radius * func_801783C8(angle)) >> 12);
    prim->y1 = centre_y + ((radius * func_801782FC(angle)) >> 12);
    phase = next;
    prim->x2 = centre_x + ((radius * func_801783C8((u16)next)) >> 12);
    prim->y2 = centre_y + ((radius * func_801782FC((u16)next)) >> 12);
    prim->r0 = centre_shade;
    prim->g0 = 0;
    prim->b0 = 0;
    prim->r1 = arc_shade;
    prim->g1 = 0;
    prim->b1 = 0;
    prim->r2 = arc_shade;
    prim->g2 = 0;
    prim->b2 = 0;
    GpuAppendPrim(1, 0x1C);
  } while ((u16)next < 0x1000);
}
