#include "bof3/bof3.h"
#include "gpu/prim.h"

void func_8017A97C(void* arg0);
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* @source 0x801F4138
 * @behavior Overlay radial fade disc: arms the shared draw mode for the
 *           texture page GetTPage(0, 1, 0x380, 0x100) on the primitive cursor
 *           g_PrimCursor with dithering off and frame-buffer dithering on,
 *           queues the 0xC-byte draw-mode primitive at ordering table index 1
 *           through the shared append func_8014E5A0, and then stamps a fan of
 *           thirty-two translucent Gouraud triangles into that cursor. It
 *           clamps both shade arguments into 0x00..0xFF the way the sibling
 *           emitters of this tree do, negative to zero and anything from 0x100
 *           to 0xFF, and tints the centre vertex with the fourth argument and
 *           both arc vertices with the fifth. Every triangle keeps its first
 *           vertex on the passed centre pair, and the phase starts at zero and
 *           advances by 0x80 per triangle through a word accumulator truncated
 *           to an unsigned halfword at each use, so thirty-two wedges of 0x80
 *           close the whole 0x1000 turn. Each arc vertex offsets the centre by
 *           the radius argument scaled through the shared rotation helpers
 *           func_801783C8 for x and func_801782FC for y and shifted down by
 *           twelve: the wedge's first arc vertex takes the phase kept from the
 *           previous step and the freshly advanced one its second, so
 *           neighbouring wedges meet at the shared sample. Each primitive is
 *           marked semi-transparent, shaped as a POLY_G3 and appended at the
 *           same ordering table index as a 0x1C-byte packet. Takes the centre
 *           pair, the radius and the two shades and returns nothing; the
 *           0x40-byte frame keeps $ra, $fp and $s0-$s7 across the calls.
 *
 * Evidence from the original bytes at 0x801F4138: addiu $sp,$sp,-0x40 with sw
 * $fp,0x38, sw $s7,0x34, sw $s0,0x18 and sw $ra,0x3C, the argument copies
 * addu $fp,$a0 / addu $s7,$a1 / addu $s0,$a2 / addu $v1,$a3, the two
 * sign-extension pairs sll $a3,16 and sra $v0,$a3,16 for the fourth argument
 * and sll $v0,$s0,16 and sra $s3,$v0,16 for the third one at the loop head, and
 * lhu $a0,0x50($sp) with the same sll/sra pair for the stacked fifth argument -
 * the sign-extension idiom the exact sibling
 * advanceHalfword2eBy0aAndSetState1To2At191ThenDrawHalfword2eWorld04Area17213_801F4DE0
 * of this target shows on its own lhu halfword, so the first four arguments are
 * signed halfwords and the stacked fifth an unsigned halfword narrowed to a
 * signed one for its clamp. Both call sites agree: 0x801F356C inside
 * func_801F34D8 loads the centre pair with lh from work record 0x1F800044
 * offsets 0x2E and 0x30, the radius and the centre shade from 0x32 and 0x0C,
 * and pushes the low halfword of the word at 0x10 after subtracting 0x10, while
 * 0x801F3678 inside func_801F35D8 does the same with the radius lowered by 0x14
 * and the pushed shade taken from offset 0x10. The two clamps are the bgez / j /
 * slti $v0,0x100 / bnez shape with the argument copied in the delay slot,
 * byte-identical to the clamp of the exact sibling emitSpinTriangle
 * (emi/world00/area024/14@0x801F3944). The prologue continues
 * with the four addiu loads for GetTPage, addu $a1,$zero,$zero, addiu
 * $a2,$zero,0x1 and andi $a3,$v0,0xFFFF - the zero extension of the unsigned
 * short GetTPage result - the g_PrimCursor load feeding SetDrawMode with
 * sw $zero,0x10 on its stack argument, and jal 0x8014E5A0 with addiu
 * $a1,$zero,0xC. The loop reloads g_PrimCursor, calls 0x8017A97C and
 * SetSemiTrans with one, stores the centre at +0x8 and +0xA, the first arc
 * vertex at +0x10 and +0x12, the second at +0x18 and +0x1A, the colour triples
 * at +0x04, +0x0C and +0x14, and closes with jal 0x8014E5A0 with
 * addiu $a1,$zero,0x1C and andi $v0,$s6,0xFF / sltiu $v0,0x20 / bnez with nop
 * in its delay slot, so the counter is a byte and the trip count 0x20.
 *
 * Shape evidence: the exact sibling drawTranslucentFanOfThirtyTwoGouraudTriangles
 * AroundScreenBottomCenter (emi/scenario/sce15ef2/00@0x801D17A0) repeats this whole
 * body - the same draw-mode arming with GetTPage(0, 1, 0x380, 0x100) and the
 * 0xC-byte append, the same cursor reload, func_8017A97C and SetSemiTrans, the
 * same six coordinate stores and three colour triples of a POLY_G3, and the same
 * 0x1C-byte append under a byte counter against 0x20 - with a fixed screen
 * centre and a 0x40 phase step in place of this body's passed centre pair and
 * 0x80 step. No payload under out/binaries holds a twin for this range: the
 * same overlay address in emi/world03/area119/13 is a different 0x80-byte
 * function.
 *
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawTranslucentRadialFanOfThirtyTwoGouraudTrianglesTintedByCenterAndArcShadeWorld04Area17213_801F4138(
    s16 centre_x, s16 centre_y, s16 radius, s16 centre_shade, u16 arc_shade) {
  POLY_G3* prim;
  u16 angle;
  s32 phase;
  u8 centre_color;
  u8 arc_color;
  u8 i;

  if (centre_shade < 0) {
    centre_color = 0;
  } else {
    centre_color = centre_shade;
    if (centre_shade >= 0x100) {
      centre_color = 0xFF;
    }
  }
  if ((s16)arc_shade < 0) {
    arc_color = 0;
  } else {
    arc_color = arc_shade;
    if ((s16)arc_shade >= 0x100) {
      arc_color = 0xFF;
    }
  }

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, GetTPage(0, 1, 0x380, 0x100), 0);
  GpuAppendPrim(1, 0xC);

  phase = 0;
  i = 0;
  do {
    prim = (POLY_G3*)g_PrimCursor;
    func_8017A97C(prim);
    SetSemiTrans(prim, 1);
    angle = phase;
    prim->x0 = centre_x;
    prim->y0 = centre_y;
    prim->x1 = centre_x + ((radius * func_801783C8(angle)) >> 12);
    prim->y1 = centre_y + ((radius * func_801782FC(angle)) >> 12);
    phase += 0x80;
    angle = phase;
    prim->x2 = centre_x + ((radius * func_801783C8(angle)) >> 12);
    prim->y2 = centre_y + ((radius * func_801782FC(angle)) >> 12);
    prim->r0 = centre_color;
    prim->g0 = centre_color;
    prim->b0 = centre_color;
    prim->r1 = arc_color;
    prim->g1 = arc_color;
    prim->b1 = arc_color;
    prim->r2 = arc_color;
    prim->g2 = arc_color;
    prim->b2 = arc_color;
    GpuAppendPrim(1, 0x1C);
    i += 1;
  } while (i < 0x20);
}
