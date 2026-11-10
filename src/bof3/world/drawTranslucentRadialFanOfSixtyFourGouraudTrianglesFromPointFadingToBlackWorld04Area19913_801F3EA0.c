#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared phase samplers reached through this target's generated
 * undefined_funcs_auto.txt rows; both return a 12-bit fixed-point sample of one
 * full turn. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Cursor stamping helper the whole draw family calls before SetSemiTrans, bound
 * by this target's generated undefined_funcs_auto.txt row. */
void func_8017A97C(void* arg0);

/* @source 0x801F3EA0
 * @behavior Overlay radial fade fan: stamps the primitive cursor g_PrimCursor
 *           and switches semi-transparency on, then emits sixty-four
 *           translucent Gouraud triangles that all share the passed point as
 *           their first vertex and close a whole 0x1000 turn between them, so
 *           together they fill a disc of the passed radius. The phase is one
 *           left shift by six of the triangle counter, kept as a signed word
 *           and resampled after the counter steps, so the wedges are 0x40 apart
 *           and the last one ends where the first began. Each arc vertex
 *           offsets the passed point by the sampled rotation scaled by the
 *           radius argument and divided by 0x1000, the centre vertex taking the
 *           phase of the current step and the third vertex the phase of the
 *           next one. The first vertex takes the shade argument on all three
 *           channels and both arc vertices are cleared, giving the disc a shade
 *           to black gradient. The 0x1C-byte POLY_G3 is appended at ordering
 *           table index 4. Takes the centre pair, the radius and the shade and
 *           returns nothing; the 0x30-byte frame keeps $ra and $s0-$s6 across
 *           the calls.
 *
 * Evidence from the original bytes at 0x801F3EA0: addiu $sp,$sp,-0x30 with sw
 * $s4,0x20, sw $s5,0x24, sw $s3,0x1C, sw $s6,0x28, sw $s2,0x18 and sw
 * $ra,0x2C, the argument copies addu $s4,$a0 / addu $s5,$a1 / addu $s3,$a2 /
 * addu $s6,$a3 with no sign-extension pair anywhere, so the four arguments are
 * full words and the s16 stores truncate them. The loop head reloads g_PrimCursor
 * and calls func_8017A97C and SetSemiTrans(prim, 1), sll $s1,$s2,6 then
 * addu $a0,$s1,$zero before the sh $s4,0x8 / sh $s5,0xA pair, mult $v0,$s3 with
 * the call result as the first operand, the bgez / addiu $v0,0xFFF / sra
 * $v0,12 rounding triple that a signed division by 0x1000 emits, and
 * addu $v0,$s4,$v0 into 0x10 and 0x12, then addiu $s2,0x1 and the same sll,
 * rounding triple and pair into 0x18 and 0x1A. The colour block is sb $s6 at 0x4,
 * 0x5 and 0x6 followed by sb $zero at 0xC, 0xD, 0xE, 0x14, 0x15 and 0x16, and
 * the primitive closes with jal 0x8014E5A0 under addiu $a0,$zero,0x4 and
 * addiu $a1,$zero,0x1C. The tail is slti $v0,$s2,0x40 / bnez to the loop head,
 * which makes the trip count 0x40 under a signed compare rather than a byte
 * counter, and the epilogue reloads $ra, $s6, $s5, $s4, $s3, $s2, $s1 and $s0
 * before addiu $sp,$sp,0x30 and jr $ra.
 *
 * Caller evidence: the only call site, 0x801F3804 inside func_801F36DC, sets
 * $a0 to 0x82, $a1 to the scratchpad work byte at 0x9 plus 0xA, $a2 to a radius
 * that starts at 0x20 and grows by twenty per step of that same work byte up to
 * a 0x1F4 cap, and $a3 to a shade clamped into 0x00..0xFF, which fixes the two
 * point words, the radius and the shade roles. Shape evidence only, not a clone
 * source: drawTranslucentRadialFanOfThirtyTwoGouraudTrianglesTintedByCenterAndArcShade
 * (emi/world04/area172/13@0x801F4138) repeats the cursor stamp, the
 * semi-transparency call, the same six coordinate stores and the 0x1C-byte
 * append of a POLY_G3, but it steps a halfword phase by 0x80 under a byte
 * counter, shifts where this body divides, and tints its arc vertices instead of
 * clearing them. No payload under out/binaries holds a byte twin for this range:
 * the 0x16C-byte span occurs once, in this target's own binary.
 *
 * Schedule evidence: the bare statement list measured 90 of 91 instructions with
 * the first phase shift hoisted above both calls, and neither statement order nor
 * a multiplied spelling of the phase moved it. Holding the phase computation, the
 * two first-vertex stores and the two first-vertex samples in one
 * single-execution scope restored the original order on both counts - the shift
 * stayed below the semi-transparency call and the argument copy came ahead of the
 * first store - and the whole body then measured byte equal. The scope holds real
 * work only; the second vertex group needs none.
 *
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentRadialFanOfSixtyFourGouraudTrianglesFromPointFadingToBlackWorld04Area19913_801F3EA0(
    s32 x, s32 y, s32 radius, s32 shade) {
  POLY_G3* prim;
  s32 angle;
  s32 i;

  i = 0;
  do {
    prim = (POLY_G3*)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);
    do {
      angle = i << 6;
      prim->x0 = x;
      prim->y0 = y;
      prim->x1 = x + func_801782FC(angle) * radius / 0x1000;
      prim->y1 = y + func_801783C8(angle) * radius / 0x1000;
    } while (0);
    i += 1;
    angle = i << 6;
    prim->x2 = x + func_801782FC(angle) * radius / 0x1000;
    prim->y2 = y + func_801783C8(angle) * radius / 0x1000;
    prim->r0 = shade;
    prim->g0 = shade;
    prim->b0 = shade;
    prim->r1 = 0;
    prim->g1 = 0;
    prim->b1 = 0;
    prim->r2 = 0;
    prim->g2 = 0;
    prim->b2 = 0;
    GpuAppendPrim(4, 0x1C);
  } while (i < 0x40);
}
