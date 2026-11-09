#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F9470
 * @behavior Eight-iteration translucent Gouraud fan emitter. Each iteration
 * re-reads the primitive published at g_PrimCursor, stamps it through
 * func_8017A97C, marks it semi-transparent through SetSemiTrans with one, and
 * writes the caller centre pair (read as two unsigned halfwords through the
 * first argument pointer) into the first vertex at +0x08 and +0x0A. The second
 * vertex pair at +0x10 and +0x12 is that centre offset by the radius-scaled
 * samples of the running phase: x takes func_801783C8 and y takes
 * func_801782FC, each product with the signed halfword scale shifted right 12,
 * and the third pair at +0x18 and +0x1A repeats it with the phase advanced by
 * 0x100, so the triangles of neighbouring iterations meet at the shared sample
 * and the eight of them span 0x800 of the 0x1000 phase circle. The first
 * vertex carries the tint byte in all three channels and both arc vertices
 * carry black, so the fan fades from the tint at the centre to the rim. The
 * packet is then appended through func_8014E5A0 with allocation class 1 and
 * size class 0x1C, the POLY_G3 length. It takes the centre pointer, a signed
 * scale, a signed phase that it advances in place and an unsigned tint byte,
 * returns nothing, and keeps a 0x30-byte frame for $ra and $s0-$s6 across the
 * sampler calls.
 *
 * Evidence: the two call sites in the neighbour func_801F90C8 (an asm row when
 * this body was seeded) pass the centre as `addiu $a0, $s3, 0x10`, the scale as
 * an `lh` halfword load from `+0x16`, the phase as an `lhu` halfword from
 * `+0x14` offset by +/-0x400 and masked with 0xFFFF, and the tint as
 * `andi $a3, $s4, 0xFF`, which fixes the pointer, signed-halfword,
 * word phase and byte argument widths. The scale is sign-extended once before
 * the loop by the `sll 16`/`sra 16` pair, so it is carried as a word and
 * narrowed to the signed halfword local. The payload block measured
 * byte-identical to the rows at 0x801F8F90 of scena07/00 and 0x801F3600 of
 * world02/area088/13, which were asm rows when this body was seeded, so the
 * clone lever was evidence only: those spans belong to other targets and were
 * left untouched.
 *
 * Measured shapes: advancing the phase parameter in place, with no separate
 * word local, is what places the two argument-register saves in the original
 * order (89/89 instead of 87/89 at the unchanged 356 bytes). The colour
 * statements are chained, which emits each triple in reverse field order, and
 * the r2 triple is written before the r1 triple, which reproduces the
 * original store schedule exactly; separate ascending assignments measure
 * 80/89.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudFanOfEightTrianglesAroundGivenCenterFadingFromTintToBlackScenarioScena0800_801F9470(
    u16* center, s32 scale, s32 phase, u8 tint) {
  POLY_G3* prim;
  s16      radius;
  u16      angle;
  u8       i;

  radius = scale;
  i = 0;
  do {
    prim = (POLY_G3*)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);
    angle = phase;
    prim->x0 = center[0];
    prim->y0 = center[1];
    prim->x1 = center[0] + ((radius * func_801783C8(angle)) >> 12);
    prim->y1 = center[1] + ((radius * func_801782FC(angle)) >> 12);
    phase += 0x100;
    angle = phase;
    prim->x2 = center[0] + ((radius * func_801783C8(angle)) >> 12);
    prim->y2 = center[1] + ((radius * func_801782FC(angle)) >> 12);
    prim->r0 = prim->g0 = prim->b0 = tint;
    prim->r2 = prim->g2 = prim->b2 = 0;
    prim->r1 = prim->g1 = prim->b1 = 0;
    func_8014E5A0(1, 0x1C);
    i++;
  } while (i < 8);
}
