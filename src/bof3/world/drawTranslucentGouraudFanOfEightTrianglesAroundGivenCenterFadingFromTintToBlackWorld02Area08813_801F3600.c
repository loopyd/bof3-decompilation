#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared EXE helpers this draw pass reaches: the engine primitive stamper
 * 0x8017A97C and the sampler pair 0x801782FC and 0x801783C8 that the running
 * phase feeds. They live outside this EMI payload, so this source only declares
 * their prototypes. */
extern s32  func_801782FC(s32 phase); /* @source 0x801782FC @kind function */
extern s32  func_801783C8(s32 phase); /* @source 0x801783C8 @kind function */
extern void func_8017A97C(void* prim); /* @source 0x8017A97C @kind function */

/* @source 0x801F3600
 * @behavior Eight-iteration translucent Gouraud fan emitter. Each iteration
 *           re-reads the primitive published at g_PrimCursor, stamps it through
 *           func_8017A97C, marks it semi-transparent through SetSemiTrans with
 *           one, and writes the caller centre pair (read as two unsigned
 *           halfwords through the first argument pointer) into the first vertex
 *           at +0x08 and +0x0A. The second vertex pair at +0x10 and +0x12 is
 *           that centre offset by the radius-scaled samples of the running
 *           phase: x takes func_801783C8 and y takes func_801782FC, each
 *           product with the signed halfword scale shifted right 12, and the
 *           third pair at +0x18 and +0x1A repeats it with the phase advanced by
 *           0x100, so the triangles of neighbouring iterations meet at the
 *           shared sample and the eight of them span 0x800 of the 0x1000 phase
 *           circle. The first vertex carries the tint byte in all three
 *           channels and both arc vertices carry black, so the fan fades from
 *           the tint at the centre to the rim. The packet is then appended
 *           through func_8014E5A0 with allocation class 1 and size class 0x1C,
 *           the POLY_G3 length. It takes the centre pointer, a signed scale, a
 *           signed phase that it advances in place and an unsigned tint byte,
 *           returns nothing, and keeps a 0x30-byte frame for $ra and $s0-$s6
 *           across the sampler calls.
 *
 * Clone evidence: the 0x164 bytes of this span are byte-identical to the
 * payload of the exact drawTranslucentGouraudFanOfEightTrianglesAroundGivenCenter
 * FadingFromTintToBlackScenarioScena0800_801F9470 in emi/scenario/scena08/00
 * (payload cmp of 0xA00 in this target's binary against 0x2870 of
 * out/binaries/emi/scenario/scena08/00.bin, both sha256
 * c3782679af19bb2628e59634a280182e20cf15adace57675c60541a4d490b199), so the
 * body below is that reviewed seed with this payload's own label. Every call in
 * the span reaches a main-executable address, so no word of the two spans
 * differs.
 *
 * Measured shapes (carried from the seed): advancing the phase parameter in
 * place, with no separate word local, places the two argument-register saves in
 * the original order; the colour statements are chained, which emits each
 * triple in reverse field order, and the r2 triple is written before the r1
 * triple, which reproduces the original store schedule.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentGouraudFanOfEightTrianglesAroundGivenCenterFadingFromTintToBlackWorld02Area08813_801F3600(
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
    GpuAppendPrim(1, 0x1C);
    i++;
  } while (i < 8);
}
